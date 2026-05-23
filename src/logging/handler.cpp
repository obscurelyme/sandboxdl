#include "logging/handler.hpp"
#include <fmt/base.h>

namespace Logging {
void Handler(void *userdata, int category, SDL_LogPriority priority,
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
} // namespace Logging
