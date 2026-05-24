#pragma once

#include "platform/spritesheet.hpp"
#include <SDL3/SDL_rect.h>

namespace Game {
class Bumper {
public:
  Bumper() = delete;
  explicit Bumper(const Sprites::Sprite &, SDL_FPoint initPosition);

  void update(float deltaTime);
  void draw(SDL_Renderer *renderer) const;
  SDL_FRect collider() const;

private:
  Sprites::Sprite sprite;
  float speed = 35;
  SDL_FPoint position;
  SDL_FRect bounds;
};

} // namespace Game