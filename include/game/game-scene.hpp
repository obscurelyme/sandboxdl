#pragma once

#include "game/background.hpp"
#include "game/ball.hpp"
#include "game/bumper.hpp"
#include "game/player-lives.hpp"
#include "platform/audio.hpp"
#include "platform/physics.hpp"
#include "platform/scene.hpp"
#include <memory>

namespace Game {
class GameScene : public Scene::IScene {
public:
  GameScene() = default;

  void onEnter() override;
  void onExit() override;
  void handleEvent(const SDL_Event &event) override;
  void update(float deltaTime, const UI::InputContext &ctx) override;
  void fixedUpdate(float deltaTime) override;
  void draw(SDL_Renderer *renderer, float alpha) override;

private:
  void buildPausedLayer(Sprites::SpriteSheet *);
  void buildGameOverLayer(Sprites::SpriteSheet *);

  void handleRetryBtnClick();
  void handleResumeBtnClick();
  void handleQuitBtnClick();

  void handleBtnHover();
  void _onEnter();

  bool paused{false};
  bool gameOver{false};
  Sprites::Sprite heart;
  Bumper bumper;
  Ball ball;
  Background background;
  std::unique_ptr<Audio::Sound> pauseSound{nullptr};
  std::unique_ptr<Audio::Sound> unpauseSound{nullptr};
  std::unique_ptr<Audio::Sound> lostLifeSound{nullptr};
  std::unique_ptr<Audio::Sound> gameOverSound{nullptr};
  std::unique_ptr<Audio::Sound> gameWinSound{nullptr};
  std::unique_ptr<Audio::Sound> btnClickSound{nullptr};
  UI::Layer uiLayer;
  UI::Layer pausedLayer;
  UI::Layer gameOverLayer;
  PlayerLives lives;
  std::shared_ptr<Physics::CircleCollider> circle{nullptr};
  std::shared_ptr<Physics::ChainCollider> ramp{nullptr};
};
} // namespace Game
