#pragma once

#include <SDL3/SDL_render.h>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Sprites {

class Sprite; // forward declaration

class SpriteSheet {
public:
  SpriteSheet(const std::string &name, SDL_Renderer *renderer);
  SpriteSheet(const SpriteSheet &) = delete;
  SpriteSheet &operator=(const SpriteSheet &) = delete;
  ~SpriteSheet();

  /**
   * Provided a valid file path to an xml file, this function will read the
   * the XML and then construct a SpriteSheet from which one can create new
   * sprites whilst saving on GPU memory.
   */
  void loadSpriteSheet(SDL_Renderer *renderer, const std::string &filePath);
  Sprite getSprite(std::string_view name) const;
  SDL_Texture *getTexture() const;

private:
  SpriteSheet();

  std::string name;
  SDL_Texture *texture;
  std::unordered_map<std::string, SDL_FRect> spriteFrames;
};

class Sprite {
public:
  Sprite() = default;
  Sprite(const SpriteSheet *, SDL_FRect coords, SDL_FRect dest);

  SDL_FRect dest; // where on the screen
  float scaleX = 1.0f;
  float scaleY = 1.0f;

  float width() const;
  float height() const;
  void colorMod(float red, float green, float blue);
  void alphaMod(float alpha);
  void draw(SDL_Renderer *) const;

private:
  const SpriteSheet *sheet;
  SDL_FRect frame;
  int alpha;
  int r;
  int g;
  int b;
};

class Manager {
public:
  static void SetRenderer(SDL_Renderer *renderer);
  static void LoadSpriteSheet(const std::string &name);
  static void Clear();
  static SpriteSheet *GetSpriteSheet(const std::string &name);

private:
  static SDL_Renderer *spriteRenderer;
  static std::unordered_map<std::string, std::unique_ptr<SpriteSheet>>
      spriteSheets;
};
} // namespace Sprites