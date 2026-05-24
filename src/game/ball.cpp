#include "game/ball.hpp"
#include "game/bricks.hpp"
#include <SDL3/SDL_stdinc.h>

namespace Game {
Ball::Ball(const Sprites::Sprite &sprite, SDL_FPoint initPosition)
    : speed(100), sprite(sprite), position(initPosition) {
  collider = {
      .x = position.x,
      .y = position.y,
      .w = sprite.width(),
      .h = sprite.height(),
  };
  bounds = {
      .x = 50 + sprite.width() * 0.5f,
      .y = 30 + sprite.height() * 0.5f,
      .w = 750 - sprite.width() * 0.5f,
      .h = 420 - sprite.height() * 0.5f,
  };

  direction.x = (SDL_randf() * 1.6f) - 0.8f;
  direction.y = -1.0f;
  float len = SDL_sqrtf(direction.x * direction.x + direction.y * direction.y);
  direction.x /= len;
  direction.y /= len;
}

void Ball::update(float deltaTime, const SDL_FRect &bumperCollider) {
  position.x += direction.x * speed * deltaTime;
  position.y += direction.y * speed * deltaTime;

  // Bounds
  if (position.x <= bounds.x) {
    position.x = bounds.x;
    direction.x = SDL_fabsf(direction.x);
  }
  if (position.x >= bounds.w) {
    position.x = bounds.w;
    direction.x = -SDL_fabsf(direction.x);
  }
  if (position.y <= bounds.y) {
    position.y = bounds.y;
    direction.y = SDL_fabsf(direction.y);
  }
  if (position.y >= bounds.h) {
    position.y = bounds.h;
    direction.y = -SDL_fabsf(direction.y);
  }

  collider = {
      .x = position.x,
      .y = position.y,
  };

  if (SDL_HasRectIntersectionFloat(&collider, &bumperCollider)) {
    direction.y = -SDL_fabsf(direction.y);
    position.y = bumperCollider.y - sprite.height();
  }

  Bricks::CheckCollisions(collider, direction);

  sprite.dest.x = position.x;
  sprite.dest.y = position.y;
}

void Ball::draw(SDL_Renderer *renderer) const { sprite.draw(renderer); }
} // namespace Game