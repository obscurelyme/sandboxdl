#pragma once

#include "game/background.hpp"
#include "game/ball.hpp"
#include "game/bumper.hpp"
#include "game/player-lives.hpp"
#include "platform/audio.hpp"
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
  void draw(SDL_Renderer *renderer) override;

private:
  bool paused{false};
  Sprites::Sprite heart;
  Bumper bumper;
  Ball ball;
  Background background;
  std::unique_ptr<Audio::Sound> lostLifeSound{nullptr};
  std::unique_ptr<Audio::Sound> gameOverSound{nullptr};
  UI::Layer uiLayer;
  PlayerLives lives;
};
} // namespace Game