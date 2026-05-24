#pragma once

#include "platform/spritesheet.hpp"

namespace Game {
class Ball {
public:
  Ball(const Sprites::Sprite &sprite, SDL_FPoint initPosition);

  void update(float deltaTime, const SDL_FRect &bumperCollider);
  void draw(SDL_Renderer *renderer) const;

  int getLives() const { return lives; }

private:
  void reset();

  int lives;
  float speed;
  Sprites::Sprite sprite;
  SDL_FPoint position;
  SDL_FRect bounds;
  SDL_FRect collider;
  SDL_FPoint direction;
};
} // namespace Game