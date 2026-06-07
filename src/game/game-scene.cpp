#include "game/game-scene.hpp"
#include "platform/events.hpp"
#include "platform/input.hpp"
#include "platform/physics.hpp"
#include "platform/profiler.hpp"
#include <box2d/box2d.h>
#include <box2d/math_functions.h>
#include <cmath>

namespace Game {
b2BodyId groundId;
b2BodyId topId;
b2BodyId leftId;
b2BodyId rightId;
const float FULL_WIDTH_UNITS = Physics::toUnits(800);
const float FULL_HEIGHT_UNITS = Physics::toUnits(450);
const float HALF_WIDTH_UNITS = Physics::toUnits(800) * 0.5f;
const float HALF_HEIGHT_UNITS = Physics::toUnits(450) * 0.5f;

std::vector<SDL_FPoint> makeArcPoints(const SDL_FPoint &center,
                                      float startAngleRadians,
                                      float sweepAngleRadians,
                                      float arcLengthPixels, int pointCount) {
  std::vector<SDL_FPoint> points;
  if (pointCount < 2) {
    return points;
  }

  const float absSweep = std::fabs(sweepAngleRadians);
  if (absSweep <= 0.0001f || arcLengthPixels <= 0.0f) {
    return points;
  }

  const float radius = arcLengthPixels / absSweep;
  points.reserve(static_cast<size_t>(pointCount));

  for (int i = 0; i < pointCount; i++) {
    const float t = static_cast<float>(i) / static_cast<float>(pointCount - 1);
    const float theta = startAngleRadians + (sweepAngleRadians * t);
    points.push_back(SDL_FPoint{
        .x = center.x + radius * std::cos(theta),
        .y = center.y + radius * std::sin(theta),
    });
  }

  return points;
}

std::vector<SDL_FPoint> makeArcPointsFromStartAndTangent(
    const SDL_FPoint &startPoint, float startTangentAngleRadians,
    float sweepAngleRadians, float arcLengthPixels, int pointCount) {
  std::vector<SDL_FPoint> points;
  if (pointCount < 2) {
    return points;
  }

  const float absSweep = std::fabs(sweepAngleRadians);
  if (absSweep <= 0.0001f || arcLengthPixels <= 0.0f) {
    return points;
  }

  const float radius = arcLengthPixels / absSweep;
  const float turnSign = sweepAngleRadians >= 0.0f ? 1.0f : -1.0f;

  const float tangentX = std::cos(startTangentAngleRadians);
  const float tangentY = std::sin(startTangentAngleRadians);

  // Rotate tangent by +90 deg for a left normal; use turnSign to select side.
  const float normalX = -tangentY * turnSign;
  const float normalY = tangentX * turnSign;

  const SDL_FPoint center{
      .x = startPoint.x + normalX * radius,
      .y = startPoint.y + normalY * radius,
  };

  const float startRadialAngle =
      std::atan2(startPoint.y - center.y, startPoint.x - center.x);

  points.reserve(static_cast<size_t>(pointCount));
  for (int i = 0; i < pointCount; i++) {
    const float t = static_cast<float>(i) / static_cast<float>(pointCount - 1);
    const float theta = startRadialAngle + (sweepAngleRadians * t);
    points.push_back(SDL_FPoint{
        .x = center.x + radius * std::cos(theta),
        .y = center.y + radius * std::sin(theta),
    });
  }

  return points;
}

void GameScene::_onEnter() {
  // SDL_HideCursor();
  pauseSound = std::make_unique<Audio::Sound>("game-pause");
  unpauseSound = std::make_unique<Audio::Sound>("game-unpause");
  lostLifeSound = std::make_unique<Audio::Sound>("lost-life");
  gameOverSound = std::make_unique<Audio::Sound>("game-over");
  gameWinSound = std::make_unique<Audio::Sound>("game-win");
  btnClickSound = std::make_unique<Audio::Sound>("button-hover");
  auto *sheet = Sprites::Manager::GetSpriteSheet("breakout-spritesheet");

  background = Background{sheet->getSprite("background-brick")};
  lives = PlayerLives{sheet->getSprite("heart")};

  uiLayer.add<UI::Backdrop>(SDL_Color{
      .r = 0,
      .g = 0,
      .b = 0,
      .a = 150,
  });
  buildPausedLayer(sheet);
  buildGameOverLayer(sheet);

  SDL_FPoint initBumperPosition{
      .x = 800.f / 2,
      .y = 400,
  };
  bumper = Bumper{sheet->getSprite("blue-bumper"), initBumperPosition};

  SDL_FPoint initBallPosition{
      .x = 800.f / 2,
      .y = 300,
  };
  ball = Ball{sheet->getSprite("gold-ball"), initBallPosition};
}

void GameScene::onEnter() {
  SDL_PROFILE_ZONE("Game::GameScene::onEnter");
  _onEnter();

  Physics::World::Create();
  auto worldId = Physics::World::Id();

  // Bottom
  b2BodyDef groundBodyDef = b2DefaultBodyDef();
  groundBodyDef.position = Physics::toB2Vec2(SDL_FPoint{
      .x = 400,
      .y = 400,
  });
  groundBodyDef.name = "Ground";
  groundId = b2CreateBody(worldId, &groundBodyDef);

  b2ShapeDef groundShapeDef = b2DefaultShapeDef();
  b2Polygon groundBox = b2MakeBox(Physics::toUnits(400), Physics::toUnits(10));
  b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

  // Top
  b2BodyDef topBodyDef = b2DefaultBodyDef();
  topBodyDef.position = Physics::toB2Vec2(SDL_FPoint{
      .x = 400,
      .y = 20,
  });
  topBodyDef.name = "Ceiling";
  topId = b2CreateBody(worldId, &topBodyDef);

  b2ShapeDef topShapeDef = b2DefaultShapeDef();
  b2Polygon topBox = b2MakeBox(8.0f, 0.2f);
  b2CreatePolygonShape(topId, &topShapeDef, &topBox);

  // Left
  b2BodyDef leftBodyDef = b2DefaultBodyDef();
  leftBodyDef.position =
      b2Vec2{.x = Physics::toUnits(100), .y = HALF_HEIGHT_UNITS};
  leftBodyDef.name = "Left Wall";
  leftId = b2CreateBody(worldId, &leftBodyDef);

  b2ShapeDef leftShapeDef = b2DefaultShapeDef();
  b2Polygon leftBox = b2MakeBox(0.2f, HALF_HEIGHT_UNITS);
  b2CreatePolygonShape(leftId, &leftShapeDef, &leftBox);

  // Right
  b2BodyDef rightBodyDef = b2DefaultBodyDef();
  rightBodyDef.position =
      b2Vec2{.x = Physics::toUnits(800 * .5), .y = HALF_HEIGHT_UNITS};
  rightBodyDef.name = "Right Wall";
  rightId = b2CreateBody(worldId, &rightBodyDef);

  b2ShapeDef rightShapeDef = b2DefaultShapeDef();
  b2Polygon rightBox = b2MakeBox(0.2f, HALF_HEIGHT_UNITS);
  b2CreatePolygonShape(rightId, &rightShapeDef, &rightBox);

  // Ball
  Physics::CircleColliderProps circleProps{
      .position =
          SDL_FPoint{
              .x = 275,
              .y = 200,
          },
      .type = Physics::BodyType::Dynamic,
      .center =
          SDL_FPoint{
              .x = 0,
              .y = 0,
          },
      .isBullet = true,
      .density = 1.0,
      .radius = 4,
      .bounce = 0.25f,
      .friction = 0.3,
      .rollResistance = 0.05,
      .name = "Pinball",
  };
  circle = std::make_shared<Physics::CircleCollider>(circleProps);

  const float sweepAngle = -Physics::deg2rads(110.0f);
  const float arcLength = 130.0f;
  const SDL_FPoint startPoint{
      .x = 70.0f,
      .y = 135.0f,
  };
  const float startTangentAngle = Physics::deg2rads(345.0f);
  std::vector<SDL_FPoint> points = makeArcPointsFromStartAndTangent(
      startPoint, startTangentAngle, sweepAngle, arcLength, 20);

  ramp = std::make_shared<Physics::ChainCollider>(points);

  Physics::World::AddCollider(circle);
  Physics::World::AddCollider(ramp);
}

void GameScene::onExit() {
  gameOver = false;
  paused = false;
  uiLayer.clear();
  pausedLayer.clear();
  gameOverLayer.clear();
  circle.reset();
  ramp.reset();
  Physics::World::Destroy();
  SDL_ShowCursor();
}

void GameScene::handleEvent(const SDL_Event &event) {
  if (event.type == Events::USER_PLAYER_LOST_LIFE) {
    lostLifeSound->play();
  }

  if (event.type == Events::USER_PAUSE) {
    SDL_ShowCursor();
    pauseSound->play();
    paused = true;
  }

  if (event.type == Events::USER_UNPAUSE) {
    unpauseSound->play();
    paused = false;
    // SDL_HideCursor();
  }

  if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST && !paused && !gameOver) {
    Events::Emit(Events::USER_PAUSE, nullptr, nullptr);
  }

  if (event.type == Events::USER_GAME_OVER) {
    gameOver = true;
    gameOverSound->play();
    SDL_ShowCursor();
  }

  if (event.type == Events::USER_GAME_WIN) {
    gameOver = true;
    gameWinSound->play();
    SDL_ShowCursor();
  }

  lives.handleEvent(event);
}

void GameScene::update(float deltaTime, const UI::InputContext &ctx) {
  if (Input::Manager::Keyboard().wasPressed(SDL_SCANCODE_ESCAPE) && !gameOver) {
    if (!paused) {
      Events::Emit(Events::USER_PAUSE, nullptr, nullptr);
    } else {
      Events::Emit(Events::USER_UNPAUSE, nullptr, nullptr);
    }
  }

  if (Input::Manager::Mouse().isLeftButtonPressed()) {
    // NOTE: left bumper move
    circle->applyImpulse(Math::Vec2{
        .x = -0.5,
        .y = -0.5,
    });
    SDL_LogInfo(0, "[GameScene] Mouse Pos (%.2f, %.2f)",
                Input::Manager::Mouse().mouseRenderPosition().x,
                Input::Manager::Mouse().mouseRenderPosition().y);
  }

  if (Input::Manager::Mouse().isRightButtonPressed()) {
    // NOTE: right bumper move
    circle->applyImpulse(Math::Vec2{
        .x = 0.5,
        .y = -0.5,
    });
  }

  if (Input::Manager::Keyboard().wasPressed(SDL_SCANCODE_Q)) {
    circle->setTransform(Math::Vec2{
        .x = Physics::toUnits(Input::Manager::Mouse().mouseRenderPosition().x),
        .y = Physics::toUnits(Input::Manager::Mouse().mouseRenderPosition().y),
    });
  }

  if (!paused && !gameOver) {
    // bumper.update(deltaTime);
    // ball.update(deltaTime, bumper.collider());
  }

  uiLayer.update(ctx);
  if (paused) {
    pausedLayer.update(ctx);
  }

  if (gameOver) {
    gameOverLayer.update(ctx);
  }
}

void GameScene::fixedUpdate(float deltaTime) {
  if (!paused) {
    Physics::World::Simulate(deltaTime);
  }
}

void GameScene::draw(SDL_Renderer *renderer, float alpha) {
  SDL_PROFILE_ZONE("Game::GameScene::draw");
  background.draw(renderer);
  uiLayer.draw(renderer);
  lives.draw(renderer);
  ball.draw(renderer);
  bumper.draw(renderer);

  Physics::World::DebugDraw(alpha);

  if (paused) {
    pausedLayer.draw(renderer);
  }

  if (gameOver) {
    gameOverLayer.draw(renderer);
  }
}

void GameScene::buildPausedLayer(Sprites::SpriteSheet *sheet) {
  pausedLayer.add<UI::Backdrop>(SDL_Color{.r = 0, .g = 0, .b = 0, .a = 100});
  auto *resumeBtn = pausedLayer.add<UI::Button>(
      UI::ButtonProps{
          .bounds = SDL_FRect{.x = 352, .y = 200, .w = 96, .h = 32},
          .paddingX = 6,
          .paddingY = 6,
          .sprite = sheet->getSprite("ui-button"),
      },
      UI::TextProps{
          .label = "Resume",
          .fontName = "Tiny5",
      });
  auto *quitBtn = pausedLayer.add<UI::Button>(
      UI::ButtonProps{
          .bounds = SDL_FRect{.x = 352, .y = 240, .w = 96, .h = 32},
          .paddingX = 6,
          .paddingY = 6,
          .sprite = sheet->getSprite("ui-button"),
      },
      UI::TextProps{
          .label = "Quit",
          .fontName = "Tiny5",
      });

  resumeBtn->onHover = [this] { handleBtnHover(); };
  quitBtn->onHover = [this] { handleBtnHover(); };

  resumeBtn->onPressed = [this] { handleResumeBtnClick(); };
  quitBtn->onPressed = [this] { handleQuitBtnClick(); };
}

void GameScene::buildGameOverLayer(Sprites::SpriteSheet *sheet) {
  gameOverLayer.add<UI::Backdrop>(SDL_Color{.r = 0, .g = 0, .b = 0, .a = 100});
  auto *retryBtn = gameOverLayer.add<UI::Button>(
      UI::ButtonProps{
          .bounds = SDL_FRect{.x = 352, .y = 200, .w = 96, .h = 32},
          .paddingX = 6,
          .paddingY = 6,
          .sprite = sheet->getSprite("ui-button"),
      },
      UI::TextProps{
          .label = "Retry",
          .fontName = "Tiny5",
      });
  auto *quitBtn = gameOverLayer.add<UI::Button>(
      UI::ButtonProps{
          .bounds = SDL_FRect{.x = 352, .y = 240, .w = 96, .h = 32},
          .paddingX = 6,
          .paddingY = 6,
          .sprite = sheet->getSprite("ui-button"),
      },
      UI::TextProps{
          .label = "Quit",
          .fontName = "Tiny5",
      });

  retryBtn->onHover = [this] { handleBtnHover(); };
  quitBtn->onHover = [this] { handleBtnHover(); };

  retryBtn->onPressed = [this] { handleRetryBtnClick(); };
  quitBtn->onPressed = [this] { handleQuitBtnClick(); };
}

void GameScene::handleRetryBtnClick() {
  Events::Emit(Events::USER_RETRY_GAME, nullptr, nullptr);
}

void GameScene::handleResumeBtnClick() {
  paused = false;
  Events::Emit(Events::USER_UNPAUSE, nullptr, nullptr);
  unpauseSound->play();
}

void GameScene::handleQuitBtnClick() {
  Events::Emit(Events::USER_QUIT_GAME, nullptr, nullptr);
}

void GameScene::handleBtnHover() { btnClickSound->play(); }
} // namespace Game
