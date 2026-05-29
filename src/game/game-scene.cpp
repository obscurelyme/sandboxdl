#include "game/game-scene.hpp"
#include "game/bricks.hpp"
#include "platform/events.hpp"
#include "platform/input.hpp"

namespace Game {
void GameScene::onEnter() {
  lostLifeSound = nullptr;
  gameOverSound = nullptr;
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

  Game::Bricks::Create(sheet);

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

void GameScene::onExit() {
  gameOver = false;
  paused = false;
  uiLayer.clear();
  pausedLayer.clear();
  gameOverLayer.clear();
}

void GameScene::handleEvent(const SDL_Event &event) {
  if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
    paused = true;
  }

  if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
    paused = false;
  }

  if (event.type == Events::USER_GAME_OVER) {
    gameOver = true;
  }

  lives.handleEvent(event);
}

void GameScene::update(float deltaTime, const UI::InputContext &ctx) {
  if (Input::Manager::Keyboard().wasPressed(SDL_SCANCODE_ESCAPE)) {
    if (!gameOver) {
      paused = !paused;
    }
  }

  if (!paused && !gameOver) {
    bumper.update(deltaTime);
    ball.update(deltaTime, bumper.collider());
  }

  uiLayer.update(ctx);
  if (paused) {
    pausedLayer.update(ctx);
  }

  if (gameOver) {
    gameOverLayer.update(ctx);
  }
}

void GameScene::draw(SDL_Renderer *renderer) {
  background.draw(renderer);
  uiLayer.draw(renderer);
  lives.draw(renderer);
  ball.draw(renderer);
  bumper.draw(renderer);
  Game::Bricks::Draw(renderer);

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

  resumeBtn->onHover = [this] {};
  quitBtn->onHover = [this] {};

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

  retryBtn->onPressed = [this] { handleRetryBtnClick(); };
  quitBtn->onPressed = [this] { handleQuitBtnClick(); };
}

void GameScene::handleRetryBtnClick() {
  Events::Emit(Events::USER_RETRY_GAME, nullptr, nullptr);
}

void GameScene::handleResumeBtnClick() { paused = false; }

void GameScene::handleQuitBtnClick() {
  Events::Emit(Events::USER_QUIT_GAME, nullptr, nullptr);
}
} // namespace Game