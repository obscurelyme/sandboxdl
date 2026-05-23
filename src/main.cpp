#include <SDL3/SDL_error.h>
#include <fmt/base.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_log.h>
#include <fmt/format.h>

void SDL_LogHandler(void *userdata, int category, SDL_LogPriority priority, const char *message) {
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
  
  SDL_LogInfo(0, "Starting application");
  bool success;
  success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
  }

  success = SDL_SetAppMetadata("SandboxDL", "0.0.1", "com.obscure.sandboxdl");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
  }



  SDL_LogInfo(0, "Closing application");
  SDL_Quit();
  SDL_LogInfo(0, "Application closed!");

  return 0;
}