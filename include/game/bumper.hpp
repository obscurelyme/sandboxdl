#pragma once

#include "platform/spritesheet.hpp"
#include <SDL3/SDL_rect.h>

namespace Game {
class Bumper {
public:
  Bumper() = delete;
  explicit Bumper(const Sprites::Sprite &);

  void update(float deltaTime);
  void draw(SDL_Renderer *renderer) const;
  SDL_FPoint position;

private:
  Sprites::Sprite sprite;
  float speed = 50;
};

} // namespace Game