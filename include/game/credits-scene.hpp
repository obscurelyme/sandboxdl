#pragma once

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
  void draw(SDL_Renderer *renderer) override;

private:
  void handleButtonFocus();
  void handleBackBtnClick();

  UI::Layer uiLayer;
  std::unique_ptr<Audio::Sound> btnClickSound;
};
} // namespace Game