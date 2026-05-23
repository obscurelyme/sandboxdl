#pragma once

#include <SDL3/SDL_log.h>

/**
 * Logging functionalities for the application
 */
namespace Logging {
/**
 * Handler for the SDL callback
 */
void Handler(void *userdata, int category, SDL_LogPriority priority,
             const char *message);

} // namespace Logging