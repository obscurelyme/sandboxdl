#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <fmt/base.h>
#include <fmt/format.h>

void SDL_LogHandler(void *userdata, int category, SDL_LogPriority priority,
                    const char *message) {
  switch (priority) {
  case SDL_LOG_PRIORITY_CRITICAL:
    fmt::println("[SDL][CRITICAL] -- {}", message);
    return;
  case SDL_LOG_PRIORITY_DEBUG:
    fmt::println("[SDL][DEBUG] -- {}", message);
    return;
  case SDL_LOG_PRIORITY_WARN:
    fmt::println("[SDL][WARN] -- {}", message);
    return;
  case SDL_LOG_PRIORITY_ERROR:
    fmt::println("[SDL][ERROR] -- {}", message);
    return;
  case SDL_LOG_PRIORITY_INFO:
    fmt::println("[SDL][INFO] -- {}", message);
    return;
  case SDL_LOG_PRIORITY_TRACE:
    fmt::println("[SDL][TRACE] -- {}", message);
    return;
  case SDL_LOG_PRIORITY_VERBOSE:
    fmt::println("[SDL][VERBOSE] -- {}", message);
    return;
  default:
    fmt::println("[SDL][Unknown] -- {}", message);
  }
}

int main(void) {
  SDL_SetLogOutputFunction(SDL_LogHandler, nullptr);

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

  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }
  }

  SDL_LogInfo(0, "Closing application");
  SDL_Quit();
  SDL_LogInfo(0, "Application closed!");

  return 0;
}