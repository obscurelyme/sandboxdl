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
      .x = 50,
      .y = 30,
      .w = 750 - sprite.width(),
      .h = 420 - sprite.height(),
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
      .w = sprite.width(),
      .h = sprite.height(),
  };

  if (SDL_HasRectIntersectionFloat(&collider, &bumperCollider)) {
    float ballCenterX = position.x + sprite.width() * 0.5f;
    float bumperCenterX = bumperCollider.x + bumperCollider.w * 0.5f;

    float hitOffset = (ballCenterX - bumperCenterX) / (bumperCollider.w * 0.5f);

    direction.x = hitOffset; // Will be between (-1, 1)
    direction.y = -1.f;

    // Renormalize so speed stays constant
    float len =
        SDL_sqrtf(direction.x * direction.x + direction.y * direction.y);
    direction.x /= len;
    direction.y /= len;

    // position.y = bumperCollider.y - sprite.height();
  }

  Bricks::CheckCollisions(collider, direction);

  sprite.dest.x = position.x;
  sprite.dest.y = position.y;
}

void Ball::draw(SDL_Renderer *renderer) const {
  sprite.draw(renderer);
  // TODO: relocate this to a dedicated collider class
  // SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  // SDL_RenderRect(renderer, &sprite.dest);
}
} // namespace Game