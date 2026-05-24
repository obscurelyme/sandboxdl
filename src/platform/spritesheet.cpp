#include "platform/spritesheet.hpp"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_surface.h>
#include <filesystem>
#include <fmt/format.h>
#include <tinyxml2.h>

namespace Sprites {
SpriteSheet::SpriteSheet() : texture(nullptr), spriteFrames({}) {}

SpriteSheet::SpriteSheet(std::string name, SDL_Texture *texture,
                         std::unordered_map<std::string, SDL_FRect> frames)
    : name(name), texture(texture), spriteFrames(frames) {}

SpriteSheet::~SpriteSheet() {
  SDL_LogTrace(0, "Freeing texture for SpriteSheet '%s'", name.c_str());
  SDL_DestroyTexture(texture);
}

SDL_Texture *SpriteSheet::getTexture() const { return texture; }

SpriteSheet SpriteSheet::loadSpriteSheet(SDL_Renderer *renderer,
                                         std::string_view filePath) {
  SDL_LogInfo(0, "Loading SpriteSheet '%s'", std::string(filePath).c_str());
  tinyxml2::XMLDocument doc;
  std::string filePathWithExt = std::string(filePath) + ".xml";
  auto assetPath = std::filesystem::path(SDL_GetBasePath()) / "assets" /
                   "sprites" / filePathWithExt;
  auto result = doc.LoadFile(assetPath.string().c_str());
  if (result != tinyxml2::XMLError::XML_SUCCESS) {
    // NOTE: could not load the xml document, we abort
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Spritesheet asset '%s' could not be loaded",
                 filePathWithExt.c_str());
    SDL_assert_always(false);
  }

  tinyxml2::XMLElement *rootEl = doc.RootElement();

  const char *nameAttr = rootEl->Attribute("name");
  SDL_assert_always(nameAttr != nullptr &&
                    "Spritesheet XML missing 'name' attribute");
  std::string name = nameAttr;

  const char *srcAttr = rootEl->Attribute("src");
  SDL_assert_always(srcAttr != nullptr &&
                    "Spritesheet XML missing 'src' attribute");
  std::string src = srcAttr;

  auto textureAssetPath =
      std::filesystem::path(SDL_GetBasePath()) / "assets" / "sprites" / src;
  SDL_Surface *surface = SDL_LoadPNG(textureAssetPath.string().c_str());
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
  SDL_DestroySurface(surface);

  std::unordered_map<std::string, SDL_FRect> frames{};
  tinyxml2::XMLElement *frame = rootEl->FirstChildElement();

  do {
    const char *elName = frame->Attribute("name");
    float elXCoord = frame->FloatAttribute("x");
    float elYCoord = frame->FloatAttribute("y");
    float elWidth = frame->FloatAttribute("w");
    float elHeight = frame->FloatAttribute("h");

    SDL_FRect rect{
        .x = elXCoord,
        .y = elYCoord,
        .w = elWidth,
        .h = elHeight,
    };

    frames[std::string(elName)] = rect;

    frame = frame->NextSiblingElement();
  } while (frame);

  SDL_LogInfo(0, "Successfully loaded SpriteSheet '%s'",
              std::string(filePath).c_str());
  return SpriteSheet{name, texture, frames};
}

Sprite SpriteSheet::getSprite(std::string_view name) const {
  auto iterator = spriteFrames.find(std::string(name));
  if (iterator == spriteFrames.end()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Sprite '%s' not found in spritesheet",
                 std::string(name).c_str());
    // NOTE: could not load the sprite, we abort
    SDL_assert_always(false);
  }

  return Sprite{
      this,
      iterator->second,
      {.x = 0, .y = 0, .w = iterator->second.w, .h = iterator->second.h}};
}

Sprite::Sprite(const SpriteSheet *sheet, SDL_FRect coords, SDL_FRect dest)
    : sheet(sheet), frame(coords), dest(dest), r(255), g(255), b(255),
      alpha(255) {}

float Sprite::width() const { return frame.w; }

float Sprite::height() const { return frame.h; }

void Sprite::draw(SDL_Renderer *renderer) const {
  SDL_SetTextureAlphaMod(sheet->getTexture(), alpha);
  SDL_SetTextureColorMod(sheet->getTexture(), r, g, b);

  SDL_RenderTexture(renderer, sheet->getTexture(), &frame, &dest);

  SDL_SetRenderScale(renderer, 1, 1);
  SDL_SetTextureAlphaMod(sheet->getTexture(), 255);
  SDL_SetTextureColorMod(sheet->getTexture(), 255, 255, 255);
}

void Sprite::colorMod(float red, float green, float blue) {
  r = SDL_clamp(red, 0, 255);
  g = SDL_clamp(green, 0, 255);
  b = SDL_clamp(blue, 0, 255);
}

void Sprite::alphaMod(float a) { alpha = SDL_clamp(a, 0, 255); }
} // namespace Sprites