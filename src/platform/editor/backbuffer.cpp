#include "platform/editor/backbuffer.hpp"
#include "SDL3/SDL_log.h"
#include <imgui.h>

namespace Editor {
SDL_Renderer *Backbuffer::renderer = nullptr;
SDL_Texture *Backbuffer::texture = nullptr;

namespace {
float gameWidth = 800.0f;
float gameHeight = 450.0f;
float viewportWidth = 1280.0f;
float viewportHeight = 720.0f;

bool viewportHovered = false;
bool viewportFocused = false;

float imageMinX = 0.0f;
float imageMinY = 0.0f;
float imageMaxX = 0.0f;
float imageMaxY = 0.0f;
} // namespace

void Backbuffer::Init(SDL_Renderer *r) { renderer = r; }

void Backbuffer::SetViewportSize(float width, float height) {
  viewportWidth = width;
  viewportHeight = height;
}

bool Backbuffer::CreateRenderTargetTexture(float width, float height) {
  gameHeight = height;
  gameWidth = width;

  if (texture) {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_TARGET, width, height);
  if (!texture) {
    SDL_LogError(0, "[Editor::Backbuffer] Failed to create game target: %s",
                 SDL_GetError());
    return false;
  }
  SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);

  return true;
}

void Backbuffer::SetRenderTargetToTexture() {
  SDL_SetRenderTarget(renderer, texture);
  SDL_SetRenderLogicalPresentation(renderer, (int)gameWidth, (int)gameHeight,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
}

void Backbuffer::SetRenderTargetToBackbuffer() {
  SDL_SetRenderTarget(renderer, nullptr);
  SDL_SetRenderLogicalPresentation(renderer, 0, 0,
                                   SDL_LOGICAL_PRESENTATION_DISABLED);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
}

void Backbuffer::RenderTexture() {
  // TODO: make this dynamic
  ImVec2 pos = ImVec2(320, 0);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(viewportWidth, viewportHeight),
                           ImGuiCond_Always);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("Viewport", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoNavFocus);

  ImVec2 avail = ImGui::GetContentRegionAvail();
  if (avail.x < 1.0f)
    avail.x = 1.0f;
  if (avail.y < 1.0f)
    avail.y = 1.0f;

  const float sx = avail.x / gameWidth;
  const float sy = avail.y / gameHeight;
  const float s = (sx < sy) ? sx : sy;

  const ImVec2 imageSize(gameWidth * s, gameHeight * s);

  // Resize the render texture to match the viewport's actual pixel size so
  // the game is rendered at full resolution rather than being upscaled.
  float texW = 0.f, texH = 0.f;
  if (texture)
    SDL_GetTextureSize(texture, &texW, &texH);
  if ((int)imageSize.x != (int)texW || (int)imageSize.y != (int)texH) {
    SDL_DestroyTexture(texture);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_TARGET, (int)imageSize.x,
                                (int)imageSize.y);
    if (texture) {
      SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    } else {
      SDL_LogError(0, "[Editor::Backbuffer] Failed to resize game target: %s",
                   SDL_GetError());
    }
  }

  const ImVec2 cursor = ImGui::GetCursorScreenPos();
  const ImVec2 imagePos(cursor.x + (avail.x - imageSize.x) * 0.5f,
                        cursor.y + (avail.y - imageSize.y) * 0.5f);

  ImGui::SetCursorScreenPos(imagePos);
  ImGui::Image(reinterpret_cast<ImTextureID>(texture), imageSize);

  const ImVec2 min = ImGui::GetItemRectMin();
  const ImVec2 max = ImGui::GetItemRectMax();

  imageMinX = min.x;
  imageMinY = min.y;
  imageMaxX = max.x;
  imageMaxY = max.y;

  viewportHovered =
      ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
  viewportFocused =
      ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

  ImGui::End();
  ImGui::PopStyleVar(); // WindowPadding
}

bool Backbuffer::IsViewportHovered() { return viewportHovered; }
bool Backbuffer::IsViewportFocused() { return viewportFocused; }

bool Backbuffer::WindowToGame(float windowX, float windowY, float &gameX,
                              float &gameY) {
  if (windowX < imageMinX || windowX > imageMaxX || windowY < imageMinY ||
      windowY > imageMaxY) {
    return false;
  }

  const float w = imageMaxX - imageMinX;
  const float h = imageMaxY - imageMinY;
  if (w <= 0.0f || h <= 0.0f) {
    return false;
  }

  const float nx = (windowX - imageMinX) / w;
  const float ny = (windowY - imageMinY) / h;

  gameX = nx * gameWidth;
  gameY = ny * gameHeight;
  return true;
}

void Backbuffer::Shutdown() { SDL_DestroyTexture(texture); }
} // namespace Editor
