#include "game/game-scene.hpp"
#include "game/bricks.hpp"

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

void GameScene::onExit() { uiLayer.clear(); }

void GameScene::handleEvent(const SDL_Event &event) {
  if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
    paused = true;
  }

  if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
    paused = false;
  }

  lives.handleEvent(event);
}

void GameScene::update(float deltaTime, const UI::InputContext &ctx) {
  if (!paused) {
    bumper.update(deltaTime);
    ball.update(deltaTime, bumper.collider());
  }
}

void GameScene::draw(SDL_Renderer *renderer) {
  background.draw(renderer);
  uiLayer.draw(renderer);
  lives.draw(renderer);
  ball.draw(renderer);
  bumper.draw(renderer);
  Game::Bricks::Draw(renderer);
}
} // namespace Game