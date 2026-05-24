#pragma once

#include "platform/spritesheet.hpp"
#include <array>

namespace Game {
class Bricks {
public:
  Bricks();

  void draw(SDL_Renderer *renderer) const;

  static void Create(const Sprites::SpriteSheet *sheet);
  static void Draw(SDL_Renderer *renderer);

private:
  Sprites::Sprite sprite;
  SDL_FPoint position;
  bool hidden;
  static std::array<Bricks, 102> bricks;
  static std::array<std::string, 6> rowColors;
};
} // namespace Game