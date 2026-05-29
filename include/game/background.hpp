#pragma once

#include "platform/spritesheet.hpp"

namespace Game {
class Background {
public:
  Background() = default;
  explicit Background(Sprites::Sprite backgroundBrick);
  void draw(SDL_Renderer *renderer);

  Sprites::Sprite backgroundBrick;
};
} // namespace Game