#include "game/ball.hpp"
#include "game/bricks.hpp"
#include "game/bumper.hpp"
#include "logging/handler.hpp"
#include "platform/debug-gui.hpp"
#include "platform/input.hpp"
#include "platform/spritesheet.hpp"
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

int main(void) {
  SDL_SetLogOutputFunction(Logging::Handler, nullptr);

  bool success;

#ifndef NDEBUG
  success = SDL_SetHint(SDL_HINT_RENDER_GPU_DEBUG, "1");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
  }
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_TRACE);
#else
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
#endif
  success = SDL_SetHint(SDL_HINT_RENDER_DRIVER, "gpu");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
  }

  SDL_LogInfo(0, "Starting application");

  success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
  }

  success = SDL_SetAppMetadata("SandboxDL", "0.0.1", "com.obscure.sandboxdl");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
  }

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

  Sprites::SpriteSheet sheet =
      Sprites::SpriteSheet::loadSpriteSheet(renderer, "breakout-spritesheet");
  Sprites::Sprite backgroundBrick = sheet.getSprite("background-brick");
  backgroundBrick.colorMod(80, 80, 80);

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

  DebugGui::FPS fpsCounter{};
  uint64_t lastTick = SDL_GetTicks();
  bool running = true;
  bool paused = false;
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
      if (event.type == SDL_EVENT_QUIT) {
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
    }

    // Update game objects
    if (!paused) {
      bumper.update(deltaTime);
      ball.update(deltaTime, bumper.collider());
    }

    // Render frame
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    drawBackground(backgroundBrick, renderer);
    ball.draw(renderer);
    bumper.draw(renderer);
    Game::Bricks::Draw(renderer);

    SDL_RenderPresent(renderer);
  }

  SDL_LogInfo(0, "Closing application");
  SDL_Quit();
  SDL_LogInfo(0, "Application closed!");

  return 0;
}