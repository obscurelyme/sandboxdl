#include "logging/handler.hpp"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

int main(void) {
  SDL_SetLogOutputFunction(Logging::Handler, nullptr);

  bool success;

#ifndef NDEBUG
  success = SDL_SetHint(SDL_HINT_RENDER_GPU_DEBUG, "1");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
  }
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

  success =
      SDL_CreateWindowAndRenderer("SandboxDL", 800, 600, 0, &window, &renderer);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
  }

  SDL_GPUDevice *gpu = SDL_GetGPURendererDevice(renderer);
  auto gpuDriver = SDL_GetGPUDeviceDriver(gpu);
  if (!gpuDriver) {
    SDL_LogError(0, "%s", SDL_GetError());
  } else {
    SDL_LogInfo(0, "Renderer created using driver <%s>", gpuDriver);
  }

  SDL_FRect rect{.x = 0, .y = 0, .w = 25, .h = 25};
  SDL_MouseMotionEvent currentMouseMotion;

  bool running = true;
  while (running) {
    // Poll input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }

      if (event.type == SDL_EVENT_MOUSE_MOTION) {
        SDL_LogInfo(0, "Mouse Position (%.2f, %.2f)", event.motion.x,
                    event.motion.y);
        currentMouseMotion = event.motion;
      }
    }

    // Update game objects
    rect.x = currentMouseMotion.x;
    rect.y = currentMouseMotion.y;

    // Render frame
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &rect);
    SDL_RenderPresent(renderer);
  }

  SDL_LogInfo(0, "Closing application");
  SDL_Quit();
  SDL_LogInfo(0, "Application closed!");

  return 0;
}