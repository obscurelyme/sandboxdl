#include "game/game-scene.hpp"
#include "game/bricks.hpp"
#include "platform/events.hpp"

namespace Game {
void GameScene::onEnter() {
  lostLifeSound = nullptr;
  gameOverSound = nullptr;
  auto *sheet = Sprites::Manager::GetSpriteSheet("breakout-spritesheet");

  backgroundBrick = sheet->getSprite("background-brick");
  backgroundBrick.colorMod(40, 40, 40);

  heart = sheet->getSprite("heart");
  heart.scaleX = 1.5f;
  heart.scaleY = 1.5f;
  heart.dest.y = 450 - heart.height() - 10;
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
  // TODO: play some sound maybe??
}

void GameScene::handleEvent(const SDL_Event &event) {
  if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
    paused = true;
  }

  if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
    paused = false;
  }
}

void GameScene::update(float deltaTime, const UI::InputContext &ctx) {
  if (!paused) {
    bumper.update(deltaTime);
    ball.update(deltaTime, bumper.collider());
  }

  if (ball.getLives() < 0) {
    Events::Emit(Events::USER_GAME_OVER, nullptr, nullptr);
  }
}

void GameScene::draw(SDL_Renderer *renderer) {
  drawBackground(backgroundBrick, renderer);
  drawHearts(ball.getLives(), heart, renderer);
  ball.draw(renderer);
  bumper.draw(renderer);
  Game::Bricks::Draw(renderer);
}

void GameScene::drawBackground(Sprites::Sprite &sprite,
                               SDL_Renderer *renderer) {
  float columns = (800 / sprite.width()) + 1;
  float rows = 450 / sprite.height();

  for (int i = 0; i < rows; i++) {
    float offsetX = i % 2 == 0 ? 0 : sprite.width() * 0.5f;

    for (int j = 0; j < columns; j++) {
      sprite.dest.x = ((float)j * sprite.width()) - offsetX;
      sprite.dest.y = (float)i * sprite.height();
      sprite.draw(renderer);
    }
  }
}

void GameScene::drawHearts(int numLives, Sprites::Sprite &sprite,
                           SDL_Renderer *renderer) {
  const float PADDING = 4;

  for (int i = 0; i < numLives; i++) {
    sprite.dest.x = 750 - (i * (sprite.width() + PADDING));
    sprite.draw(renderer);
  }
}
} // namespace Game