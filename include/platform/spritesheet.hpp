#pragma once

#include <SDL3/SDL_render.h>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Sprites {

class Sprite; // forward declaration

class SpriteSheet {
public:
  SpriteSheet(std::string name, SDL_Texture *texture,
              std::unordered_map<std::string, SDL_FRect> frames);
  SpriteSheet(const SpriteSheet &) = delete;
  SpriteSheet &operator=(const SpriteSheet &) = delete;
  ~SpriteSheet();

  /**
   * Provided a valid file path to an xml file, this function will read the
   * the XML and then construct a SpriteSheet from which one can create new
   * sprites whilst saving on GPU memory.
   */
  static SpriteSheet loadSpriteSheet(SDL_Renderer *renderer,
                                     std::string_view filePath);
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
  Sprite(const SpriteSheet *, SDL_FRect coords, SDL_FRect dest);

  SDL_FRect dest; // where on the screen

  void draw(SDL_Renderer *) const;

private:
  const SpriteSheet *sheet;
  SDL_FRect frame;
};

} // namespace Sprites