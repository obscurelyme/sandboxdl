#pragma once

#include "game/background.hpp"
#include "platform/audio.hpp"
#include "platform/scene.hpp"
#include <memory>

namespace Game {
class CreditsScene : public Scene::IScene {
public:
  void onEnter() override;
  void onExit() override;
  void handleEvent(const SDL_Event &event) override;
  void update(float deltaTime, const UI::InputContext &ctx) override;
  void draw(SDL_Renderer *renderer, float alpha) override;

private:
  void handleButtonFocus();
  void handleBackBtnClick();

  Background background;
  UI::Layer uiLayer;
  std::unique_ptr<Audio::Sound> btnClickSound;
  SDL_Color linkHoverColor{
      .r = 64,
      .g = 175,
      .b = 255,
      .a = 255,
  };
  SDL_Color linkDefaultColor{
      .r = 255,
      .g = 255,
      .b = 255,
      .a = 255,
  };
};
} // namespace Game