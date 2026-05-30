#include "game/credits-scene.hpp"
#include "platform/events.hpp"

namespace Game {
void CreditsScene::onEnter() {
  btnClickSound = std::make_unique<Audio::Sound>("button-hover");
  auto *sheet = Sprites::Manager::GetSpriteSheet("breakout-spritesheet");

  background = Background{sheet->getSprite("background-brick")};

  uiLayer.add<UI::Backdrop>(SDL_Color{
      .r = 0,
      .g = 0,
      .b = 0,
      .a = 150,
  });
  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 75,
          .y = 25,
      },
      "Credits", UI::FontManager::GetFont("Tiny5"), 60);
  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 75,
          .y = 75,
      },
      "-----------------------", UI::FontManager::GetFont("Tiny5"), 16);

  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 75,
          .y = 115,
      },
      "Programmers:", UI::FontManager::GetFont("Tiny5"), 24);

  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 90,
          .y = 145,
      },
      "Nicola (Mackenzie) Greco", UI::FontManager::GetFont("Tiny5"), 16);

  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 75,
          .y = 200,
      },
      "Artists:", UI::FontManager::GetFont("Tiny5"), 24);

  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 90,
          .y = 230,
      },
      "Buch", UI::FontManager::GetFont("Tiny5"), 16);

  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 75,
          .y = 275,
      },
      "Sound:", UI::FontManager::GetFont("Tiny5"), 24);

  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 90,
          .y = 305,
      },
      "Kenny", UI::FontManager::GetFont("Tiny5"), 16);

  uiLayer.add<UI::Text>(
      SDL_FPoint{
          .x = 90,
          .y = 335,
      },
      "SubspaceAudio", UI::FontManager::GetFont("Tiny5"), 16);

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

void CreditsScene::onExit() { uiLayer.clear(); }

void CreditsScene::handleEvent(const SDL_Event &event) {
  // NOTE: no-op
}

void CreditsScene::update(float deltaTime, const UI::InputContext &ctx) {
  uiLayer.update(ctx);
}

void CreditsScene::draw(SDL_Renderer *renderer) {
  background.draw(renderer);
  uiLayer.draw(renderer);
}

void CreditsScene::handleButtonFocus() { btnClickSound->play(); }

void CreditsScene::handleBackBtnClick() {
  Events::Emit(Events::USER_HIDE_CREDITS, nullptr, nullptr);
}
} // namespace Game