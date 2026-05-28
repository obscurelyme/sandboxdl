#include "game/ball.hpp"
#include "game/bricks.hpp"
#include "game/bumper.hpp"
#include "logging/handler.hpp"
#include "platform/audio.hpp"
#include "platform/debug-gui.hpp"
#include "platform/events.hpp"
#include "platform/input.hpp"
#include "platform/spritesheet.hpp"
#include "platform/text.hpp"
#include "platform/ui.hpp"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

void drawBackground(Sprites::Sprite &sprite, SDL_Renderer *renderer) {
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

void drawHearts(int numLives, Sprites::Sprite &sprite, SDL_Renderer *renderer) {
  const float PADDING = 4;

  for (int i = 0; i < numLives; i++) {
    sprite.dest.x = 750 - (i * (sprite.width() + PADDING));
    sprite.draw(renderer);
  }
}

int main(void) {
  /* #region Init Logic */
  SDL_SetLogOutputFunction(Logging::Handler, nullptr);

  bool success;

#ifndef NDEBUG
  success = SDL_SetHint(SDL_HINT_RENDER_GPU_DEBUG, "1");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return 1;
  }
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_TRACE);
#else
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
#endif
  success = SDL_SetHint(SDL_HINT_RENDER_DRIVER, "gpu");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return 1;
  }

  SDL_LogInfo(0, "Starting application");

  success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return 1;
  }

  success = SDL_SetAppMetadata("SandboxDL", "0.0.1", "com.obscure.sandboxdl");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return 1;
  }

  success = UI::FontManager::Init();
  if (!success) {
    return 1;
  }
  /* #endregion */

  /* #region Window and Renderer Creation */
  SDL_Window *window;
  SDL_Renderer *renderer;

  success = SDL_CreateWindowAndRenderer("SandboxDL", 1920, 1080, 0, &window,
                                        &renderer);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
  }

  SDL_SetRenderLogicalPresentation(renderer, 800, 450,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_GPUDevice *gpu = SDL_GetGPURendererDevice(renderer);
  auto gpuDriver = SDL_GetGPUDeviceDriver(gpu);
  if (!gpuDriver) {
    SDL_LogError(0, "%s", SDL_GetError());
  } else {
    SDL_LogInfo(0, "Renderer created using driver <%s>", gpuDriver);
  }
  /* #endregion */

  UI::FontManager::SetRenderer(renderer);
  UI::FontManager::LoadFont("Tiny5");

  Sprites::SpriteSheet sheet =
      Sprites::SpriteSheet::loadSpriteSheet(renderer, "breakout-spritesheet");

  Audio::Sound *btnHoverClick = new Audio::Sound("button-hover-click");

  /* #region UI */
  UI::Layer uiLayer;
  Sprites::Sprite uiBtnSprite = sheet.getSprite("ui-button");
  Sprites::Sprite uiBtnSprite2 = sheet.getSprite("ui-button");
  Sprites::Sprite uiBtnSprite3 = sheet.getSprite("ui-button");
  uiLayer.add<UI::Backdrop>(SDL_Color{
      .r = 0,
      .g = 0,
      .b = 0,
      .a = 100,
  });
  auto *btn = uiLayer.add<UI::Button>(
      UI::ButtonProps{
          .bounds = SDL_FRect{.x = 352, .y = 200, .w = 96, .h = 32},
          .paddingX = 6,
          .paddingY = 6,
          .sprite = &uiBtnSprite,
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
          .sprite = &uiBtnSprite2,
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
          .sprite = &uiBtnSprite3,
      },
      UI::TextProps{
          .label = "Quit",
          .fontName = "Tiny5",
      });

  btn->onPressed = [] {};
  btn->onHover = [&btnHoverClick] { btnHoverClick->play(); };
  creditsBtn->onHover = [&btnHoverClick] { btnHoverClick->play(); };
  quitBtn->onHover = [&btnHoverClick] { btnHoverClick->play(); };
  btn->onBlur = [] {};
  btn->onFocus = [] {};
  quitBtn->onPressed = [] {
    Events::Emit(Events::USER_QUIT_APP, nullptr, nullptr);
  };
  /* #endregion */

  /* #region Scene */
  Sprites::Sprite backgroundBrick = sheet.getSprite("background-brick");
  backgroundBrick.colorMod(40, 40, 40);

  Sprites::Sprite heart = sheet.getSprite("heart");
  heart.scaleX = 1.5f;
  heart.scaleY = 1.5f;
  heart.dest.y = 450 - heart.height() - 10;
  Game::Bricks::Create(&sheet);

  SDL_FPoint initBumperPosition{
      .x = 800.f / 2,
      .y = 400,
  };
  Game::Bumper bumper{sheet.getSprite("blue-bumper"), initBumperPosition};

  SDL_FPoint initBallPosition{
      .x = 800.f / 2,
      .y = 300,
  };
  Game::Ball ball{sheet.getSprite("gold-ball"), initBallPosition};
  /* #endregion */

  DebugGui::FPS fpsCounter{};
  uint64_t lastTick = SDL_GetTicks();
  bool paused = false;
  UI::InputContext inputCtx;
  bool running = true;
  while (running) {
    fpsCounter.update();
    uint64_t now = SDL_GetTicks();
    float deltaTime =
        SDL_min((now - lastTick) / 1000.0f, 0.05f); // capped at 50ms
    lastTick = now;

    // Clear previous frame input
    Input::Manager::Swap();

    // Poll input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT || event.type == Events::USER_QUIT_APP) {
        running = false;
        continue;
      }

      // TODO: come up with a more decoupled way of handling this...
      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.scancode == SDL_SCANCODE_SLASH) {
          fpsCounter.tuiReport();
        }
      }

      if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
        paused = true;
        Input::Manager::Reset();
      }

      if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
        lastTick = SDL_GetTicks();
        paused = false;
      }

      Input::Manager::HandleInputEvent(renderer, event);
      inputCtx = UI::SnapshotCtx();
    }

    // UI-Layer Update
    uiLayer.update(inputCtx);

    // Update game objects
    if (!paused) {
      bumper.update(deltaTime);
      ball.update(deltaTime, bumper.collider());
    }

    // Render frame
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    drawBackground(backgroundBrick, renderer);
    drawHearts(ball.getLives(), heart, renderer);
    ball.draw(renderer);
    bumper.draw(renderer);
    Game::Bricks::Draw(renderer);

    // UI-Layer Draw
    uiLayer.draw(renderer);

    SDL_RenderPresent(renderer);
  }

  SDL_free(btnHoverClick);
  SDL_LogInfo(0, "Closing application");
  uiLayer.clear();
  UI::FontManager::Quit();
  SDL_Quit();
  SDL_LogInfo(0, "Application closed!");

  return 0;
}