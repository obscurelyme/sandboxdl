#include "platform/text.hpp"
#include <filesystem>

namespace UI {

Font::Font(std::string fontName, SDL_Renderer *renderer)
    : ttfFont(nullptr), fontRenderer(renderer) {
  std::string fontFilePathWithExt = fontName + ".ttf";
  auto assetPath = std::filesystem::path(SDL_GetBasePath()) / "assets" /
                   "fonts" / fontFilePathWithExt;
  ptSize = 16;
  ttfFont = TTF_OpenFont(assetPath.string().c_str(), ptSize);
  if (!ttfFont) {
    SDL_LogError(0, "[UI::Font] Unable to load TTF font '%s'. %s",
                 fontName.c_str(), SDL_GetError());
    SDL_assert_always(false);
  }
}

Font::~Font() { TTF_CloseFont(ttfFont); }

bool Font::enableSDF() {
  bool success = TTF_SetFontSDF(ttfFont, true);

  if (!success) {
    SDL_LogError(0, "[UI::Font] %s", SDL_GetError());
  }

  return success;
}

bool Font::disableSDF() {
  bool success = TTF_SetFontSDF(ttfFont, false);

  if (!success) {
    SDL_LogError(0, "[UI::Font] %s", SDL_GetError());
  }

  return success;
}

SDL_Color Font::defaultColor = {
    .r = 255,
    .g = 255,
    .b = 255,
    .a = 255,
};

SDL_Texture *Font::renderText(std::string text, SDL_Color *color) {
  SDL_Color c;

  if (color != nullptr) {
    c = *color;
  } else {
    c = defaultColor;
  }

  SDL_Surface *surface = TTF_RenderText_Blended(ttfFont, text.c_str(), 0, c);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(fontRenderer, surface);
  SDL_DestroySurface(surface);
  SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
  return texture;
}

SDL_Texture *Font::renderTextFitted(std::string text, SDL_Color *color,
                                    SDL_FRect bounds) {
  int w, h;
  TTF_GetStringSize(ttfFont, text.c_str(), 0, &w, &h);
  float scale = SDL_min(bounds.w / (float)w, bounds.h / (float)h);
  TTF_SetFontSize(ttfFont, ptSize * scale);
  SDL_Texture *scaledTexture = renderText(text, color);
  TTF_SetFontSize(ttfFont, ptSize);
  return scaledTexture;
}

SDL_Renderer *FontManager::fontRenderer = nullptr;
std::unordered_map<std::string, std::shared_ptr<Font>> FontManager::fonts{};

bool FontManager::Init() {
  bool success = TTF_Init();

  if (!success) {
    SDL_LogError(0, "[Text::Manager] %s", SDL_GetError());
  } else {
    int freetypeMajor;
    int freetypeMinor;
    int freetypePatch;

    TTF_GetFreeTypeVersion(&freetypeMajor, &freetypeMinor, &freetypePatch);

    SDL_LogInfo(0, "[Text::Manager] Initialized SDL_ttf");
    SDL_LogInfo(0, "[Text::Manager] SDL_ttf version - %d", TTF_Version());
    SDL_LogInfo(0, "[Text::Manager] freetype version - %d.%d.%d", freetypeMajor,
                freetypeMinor, freetypePatch);
  }

  return success;
}

void FontManager::SetRenderer(SDL_Renderer *renderer) {
  fontRenderer = renderer;
}

void FontManager::LoadFont(std::string fontName) {
  if (fontRenderer == nullptr) {
    SDL_LogError(
        0,
        "[UI::FontManager] Call to LoadFont made before fontRenderer was "
        "set. This will result in '%s' font not displaying on the screen.",
        fontName.c_str());
  }

  auto font = std::make_shared<Font>(fontName, fontRenderer);

  fonts[fontName] = font;
}

std::shared_ptr<Font> FontManager::GetFont(std::string fontName) {
  auto it = fonts.find(fontName);
  if (it == fonts.end()) {
    SDL_LogError(
        0,
        "[UI::FontManager] Font '%s' was not found in the application's font "
        "registry, please call "
        "LoadFont before attempting to use it.",
        fontName.c_str());
    SDL_assert_always(false);
  }

  return it->second;
}

void FontManager::Quit() {
  if (TTF_WasInit()) {
    for (auto &[name, font] : fonts) {
      if (font.use_count() > 1) {
        SDL_LogWarn(0, "[Text::Manager] Font '%s' still in use at Quit.",
                    name.c_str());
      }
    }

    fonts.clear();
    TTF_Quit();
  }
}

} // namespace UI