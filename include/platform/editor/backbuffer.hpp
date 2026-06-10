#pragma once

#include <SDL3/SDL_render.h>

namespace Editor {
class Backbuffer {
public:
  static void Init(SDL_Renderer *r);
  static bool CreateRenderTargetTexture(float width, float height);
  static void SetViewportSize(float width, float height);
  static void SetRenderTargetToBackbuffer();
  static void SetRenderTargetToTexture();
  static void RenderTexture();
  static void Shutdown();

  static bool IsViewportHovered();
  static bool IsViewportFocused();
  static bool WindowToGame(float windowX, float windowY, float &gameX,
                           float &gameY);

private:
  static SDL_Renderer *renderer;
  static SDL_Texture *texture;
};
} // namespace Editor
