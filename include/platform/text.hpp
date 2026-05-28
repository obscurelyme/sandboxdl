#pragma once

#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace UI {
class Font {
public:
  Font(std::string fontName, SDL_Renderer *renderer);
  ~Font();

  Font(const Font &) = delete;
  Font &operator=(const Font &) = delete;
  Font(Font &&) = default;
  Font &operator=(Font &&) = default;

  bool enableSDF();
  bool disableSDF();
  SDL_Texture *renderText(std::string text, SDL_Color *color);
  SDL_Texture *renderTextFitted(std::string text, SDL_Color *color,
                                SDL_FRect bounds);

private:
  float ptSize;
  static SDL_Color defaultColor;
  TTF_Font *ttfFont;
  SDL_Renderer *fontRenderer;
};

class FontManager {
public:
  /**
   * Initializes the text manager, must be called during application startup
   * before any other text-based functions.
   */
  static bool Init();

  static void SetRenderer(SDL_Renderer *renderer);

  /**
   * Loads a given font from the assets directory.
   */
  static void LoadFont(std::string fontName);

  static std::shared_ptr<Font> GetFont(std::string fontName);

  static void Quit();

private:
  static std::unordered_map<std::string, std::shared_ptr<Font>> fonts;
  static SDL_Renderer *fontRenderer;
};
} // namespace UI