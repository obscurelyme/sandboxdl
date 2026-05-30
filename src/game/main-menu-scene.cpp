#include "game/main-menu-scene.hpp"
#include "platform/events.hpp"

namespace Game {
void MainMenuScene::onEnter() {
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
          .x = 280,
          .y = 75,
      },
      "Breakout", UI::FontManager::GetFont("Tiny5"), 72);
  auto *playBtn = uiLayer.add<UI::Button>(
      UI::ButtonProps{
          .bounds = SDL_FRect{.x = 352, .y = 200, .w = 96, .h = 32},
          .paddingX = 6,
          .paddingY = 6,
          .sprite = sheet->getSprite("ui-button"),
      },
      UI::TextProps{
          .label = "Play",
          .fontName = "Tiny5",
      });
  auto *creditsBtn = uiLayer.add<UI::Button>(
      UI::ButtonProps{
          .bounds = SDL_FRect{.x = 352, .y = 240, .w = 96, .h = 32},
          .paddingX = 6,
          .paddingY = 6,
          .sprite = sheet->getSprite("ui-button"),
      },
      UI::TextProps{
          .label = "Credits",
          .fontName = "Tiny5",
      });
  auto *quitBtn = uiLayer.add<UI::Button>(
      UI::ButtonProps{
          .bounds = SDL_FRect{.x = 352, .y = 280, .w = 96, .h = 32},
          .paddingX = 6,
          .paddingY = 6,
          .sprite = sheet->getSprite("ui-button"),
      },
      UI::TextProps{
          .label = "Quit",
          .fontName = "Tiny5",
      });

  playBtn->onPressed = [this] { handlePlayBtnClick(); };
  playBtn->onFocus = [this] { handleButtonFocus(); };
  playBtn->onHover = [this] { handleButtonFocus(); };

  creditsBtn->onPressed = [this] { handleCreditsBtnClick(); };
  creditsBtn->onHover = [this] { handleButtonFocus(); };
  creditsBtn->onFocus = [this] { handleButtonFocus(); };

  quitBtn->onFocus = [this] { handleButtonFocus(); };
  quitBtn->onHover = [this] { handleButtonFocus(); };
  quitBtn->onPressed = [this] { handleQuitBtnClick(); };
}

void MainMenuScene::onExit() { uiLayer.clear(); }

void MainMenuScene::handleEvent(const SDL_Event &event) {
  // Note: this might just be a no-op
}

void MainMenuScene::update(float deltaTime, const UI::InputContext &ctx) {
  uiLayer.update(ctx);
}

void MainMenuScene::draw(SDL_Renderer *renderer) {
  background.draw(renderer);
  uiLayer.draw(renderer);
}

void MainMenuScene::handleButtonFocus() { btnClickSound->play(); }

void MainMenuScene::handlePlayBtnClick() {
  Events::Emit(Events::USER_PLAY_GAME, nullptr, nullptr);
}

void MainMenuScene::handleCreditsBtnClick() {
  Events::Emit(Events::USER_SHOW_CREDITS, nullptr, nullptr);
}

void MainMenuScene::handleQuitBtnClick() {
  Events::Emit(Events::USER_QUIT_APP, nullptr, nullptr);
}
} // namespace Game