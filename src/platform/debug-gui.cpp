#include "platform/debug-gui.hpp"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_timer.h>

namespace DebugGui {
FPS::FPS() : now(0), diff(0), lastTick(0), framesRendered(0), currentFps(0) {}

void FPS::tuiReport() const {
  SDL_LogDebug(0, "Current FPS [%.4f]", currentFps);
}

void FPS::update() {
  now = SDL_GetTicks();

  if (now - lastTick >= 1000) {
    lastTick = now;
    currentFps = framesRendered;
    framesRendered = 0;
  } else {
    framesRendered++;
  }
}
} // namespace DebugGui