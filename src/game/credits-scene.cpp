#include "game/credits-scene.hpp"
#include "platform/events.hpp"

namespace Game {
void CreditsScene::onEnter() {
  btnClickSound = std::make_unique<Audio::Sound>("button-hover-click");
  auto *sheet = Sprites::Manager::GetSpriteSheet("breakout-spritesheet");

  uiLayer.add<UI::Backdrop>(SDL_Color{
      .r = 0,
      .g = 0,
      .b = 0,
      .a = 100,
  });
  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 75,
          .y = 75,
      },
      "Credits", UI::FontManager::GetFont("Tiny5"), 60);
  auto *backBtn = uiLayer.add<UI::Button>(
      UI::ButtonProps{
          .bounds = SDL_FRect{.x = 75, .y = 400, .w = 96, .h = 32},
          .paddingX = 6,
          .paddingY = 6,
          .sprite = sheet->getSprite("ui-button"),
      },
      UI::TextProps{
          .label = "Back",
          .fontName = "Tiny5",
      });

  backBtn->onPressed = [this] { handleBackBtnClick(); };
  backBtn->onFocus = [this] { handleButtonFocus(); };
  backBtn->onHover = [this] { handleButtonFocus(); };
}

void CreditsScene::onExit() {
  // TODO: play some sound maybe??
}

void CreditsScene::handleEvent(const SDL_Event &event) {
  // Note: this might just be a no-op
}

void CreditsScene::update(float deltaTime, const UI::InputContext &ctx) {
  uiLayer.update(ctx);
}

void CreditsScene::draw(SDL_Renderer *renderer) {
  // TODO: draw the background as well. function is in main.cpp need to abstract
  // that and add it here...
  uiLayer.draw(renderer);
}

void CreditsScene::handleButtonFocus() { btnClickSound->play(); }

void CreditsScene::handleBackBtnClick() {
  Events::Emit(Events::USER_HIDE_CREDITS, nullptr, nullptr);
}
} // namespace Game