#pragma once

#include "platform/ui.hpp"
#include <SDL3/SDL_render.h>

namespace DebugGui {
class FPS {
public:
  FPS();

  void update();
  void draw(SDL_Renderer *renderer);
  void tuiReport() const;

private:
  float now;
  float diff;
  float lastTick;
  int framesRendered;
  float currentFps;
  UI::Text fpsText;
  std::string text;
};
} // namespace DebugGui