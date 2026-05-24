#pragma once

namespace DebugGui {
class FPS {
public:
  FPS();

  void update();
  void tuiReport() const;

private:
  float now;
  float diff;
  float lastTick;
  int framesRendered;
  float currentFps;
};
} // namespace DebugGui