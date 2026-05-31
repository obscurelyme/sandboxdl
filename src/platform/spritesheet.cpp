#include "platform/spritesheet.hpp"
#include "platform/profiler.hpp"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_surface.h>
#include <filesystem>
#include <fmt/format.h>
#include <tinyxml2.h>

namespace Sprites {
SpriteSheet::SpriteSheet(const std::string &name, SDL_Renderer *renderer) {
  SDL_PROFILE_ZONE("Sprites::SpriteSheet::Constructor");
  if (!renderer) {
    SDL_LogError(0, "[Sprites::SpriteSheet] SDL_Renderer was not set, are you "
                    "sure you called Sprites::Manager::SetRenderer?");
    SDL_assert_always(false);
  }
  loadSpriteSheet(renderer, name);
}

SpriteSheet::~SpriteSheet() {
  SDL_PROFILE_ZONE("Sprites::SpriteSheet::Destructor");
  SDL_LogTrace(0, "Freeing texture for SpriteSheet '%s'", name.c_str());
  SDL_DestroyTexture(texture);
  SDL_DestroySurface(surface);
}

SDL_Texture *SpriteSheet::getTexture() const { return texture; }

void SpriteSheet::loadSpriteSheet(SDL_Renderer *renderer,
                                  const std::string &filePath) {
  SDL_PROFILE_ZONE("Sprites::SpriteSheet::loadSpriteSheet");
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
  name = nameAttr;

  const char *srcAttr = rootEl->Attribute("src");
  SDL_assert_always(srcAttr != nullptr &&
                    "Spritesheet XML missing 'src' attribute");
  std::string src = srcAttr;

  auto textureAssetPath =
      std::filesystem::path(SDL_GetBasePath()) / "assets" / "sprites" / src;
  surface = SDL_LoadPNG(textureAssetPath.string().c_str());
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

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

  spriteFrames = frames;
}

Sprite SpriteSheet::getSprite(std::string_view name) const {
  SDL_PROFILE_ZONE("Sprites::SpriteSheet::getSprite");
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

SDL_Surface *SpriteSheet::getSurface(const std::string &name,
                                     float scale) const {
  SDL_PROFILE_ZONE("Sprites::SpriteSheet::getSurface");
  auto iterator = spriteFrames.find(std::string(name));
  if (iterator == spriteFrames.end()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Sprite '%s' not found in spritesheet",
                 std::string(name).c_str());
    // NOTE: could not load the sprite, we abort
    SDL_assert_always(false);
  }

  if (scale <= 0.0f) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Invalid sprite surface scale (%f) for '%s'; using 1.0f", scale,
                name.c_str());
    scale = 1.0f;
  }

  const SDL_FRect &frame = iterator->second;
  SDL_Rect srcRect = {
      .x = static_cast<int>(frame.x),
      .y = static_cast<int>(frame.y),
      .w = static_cast<int>(frame.w),
      .h = static_cast<int>(frame.h),
  };

  int dstWidth = static_cast<int>(frame.w * scale);
  int dstHeight = static_cast<int>(frame.h * scale);
  if (dstWidth < 1) {
    dstWidth = 1;
  }
  if (dstHeight < 1) {
    dstHeight = 1;
  }

  SDL_Surface *subSurface =
      SDL_CreateSurface(dstWidth, dstHeight, surface->format);
  if (!subSurface) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create sub-surface for sprite '%s': %s",
                 name.c_str(), SDL_GetError());
    SDL_assert_always(false);
  }

  SDL_Rect dstRect = {
      .x = 0,
      .y = 0,
      .w = dstWidth,
      .h = dstHeight,
  };

  bool blitSuccess = SDL_BlitSurfaceScaled(surface, &srcRect, subSurface,
                                           &dstRect, SDL_SCALEMODE_NEAREST);
  if (!blitSuccess) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to blit sub-surface for sprite '%s': %s", name.c_str(),
                 SDL_GetError());
    SDL_DestroySurface(subSurface);
    SDL_assert_always(false);
  }

  return subSurface;
}

Sprite::Sprite(const SpriteSheet *sheet, SDL_FRect coords, SDL_FRect dest)
    : sheet(sheet), frame(coords), dest(dest), r(255), g(255), b(255),
      alpha(255) {}

float Sprite::width() const { return frame.w * scaleX; }

float Sprite::height() const { return frame.h * scaleY; }

void Sprite::draw(SDL_Renderer *renderer) const {
  SDL_PROFILE_ZONE("Sprites::Sprite::draw");
  SDL_SetTextureAlphaMod(sheet->getTexture(), alpha);
  SDL_SetTextureColorMod(sheet->getTexture(), r, g, b);

  SDL_FRect scaledDest = {dest.x, dest.y, dest.w * scaleX, dest.h * scaleY};
  SDL_RenderTexture(renderer, sheet->getTexture(), &frame, &scaledDest);

  SDL_SetTextureAlphaMod(sheet->getTexture(), 255);
  SDL_SetTextureColorMod(sheet->getTexture(), 255, 255, 255);
}

void Sprite::colorMod(float red, float green, float blue) {
  r = SDL_clamp(red, 0, 255);
  g = SDL_clamp(green, 0, 255);
  b = SDL_clamp(blue, 0, 255);
}

void Sprite::alphaMod(float a) { alpha = SDL_clamp(a, 0, 255); }

SDL_Texture *Sprite::texture() const { return sheet->getTexture(); }

const SDL_FRect &Sprite::source() const { return frame; }

std::unordered_map<std::string, std::unique_ptr<SpriteSheet>>
    Manager::spriteSheets{};

SDL_Renderer *Manager::spriteRenderer = nullptr;

void Manager::SetRenderer(SDL_Renderer *renderer) { spriteRenderer = renderer; }

void Manager::LoadSpriteSheet(const std::string &name) {
  SDL_PROFILE_ZONE("Sprites::Manager::LoadSpriteSheet");
  auto iterator = spriteSheets.find(name);
  if (iterator != spriteSheets.end()) {
    SDL_LogWarn(
        0,
        "[Sprites::Manager] Invalid attempt to load existing spritesheet '%s'",
        name.c_str());
    return;
  }

  spriteSheets[name] = std::make_unique<SpriteSheet>(name, spriteRenderer);
}

void Manager::Clear() { spriteSheets.clear(); }

SpriteSheet *Manager::GetSpriteSheet(const std::string &name) {
  SDL_PROFILE_ZONE("Sprites::Manager::GetSpriteSheet");
  auto iterator = spriteSheets.find(name);
  if (iterator == spriteSheets.end()) {
    SDL_LogError(0, "[Sprites::Manager] Spritesheet does '%s' does not exist",
                 name.c_str());
    SDL_assert_always(false);
  }

  return iterator->second.get();
}

} // namespace Sprites
