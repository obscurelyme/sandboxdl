#pragma once

#include "game/ball.hpp"
#include "game/bumper.hpp"
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
  void drawBackground(Sprites::Sprite &, SDL_Renderer *);
  void drawHearts(int, Sprites::Sprite &, SDL_Renderer *);

  bool paused{false};
  Sprites::Sprite backgroundBrick;
  Sprites::Sprite heart;
  Bumper bumper;
  Ball ball;
  std::unique_ptr<Audio::Sound> lostLifeSound{nullptr};
  std::unique_ptr<Audio::Sound> gameOverSound{nullptr};
};
} // namespace Game