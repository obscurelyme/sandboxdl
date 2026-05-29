#include "game/ball.hpp"
#include "game/bricks.hpp"
#include "platform/events.hpp"
#include <SDL3/SDL_log.h>
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
    Events::Emit(Events::USER_PLAYER_LOST_LIFE, nullptr, nullptr);
    reset();
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

    // Center hits reduce speed (up to 15%), edge hits have no effect.
    // Floored at the base speed of 100 so it never goes below starting speed.
    float centerFactor = 1.0f - SDL_fabsf(hitOffset);
    speed = SDL_max(100.0f, speed * (1.0f - 0.15f * centerFactor));

    // NOTE: to future self, if the ball starts misbehaving in odd ways maybe
    // try uncommenting this. I didn't like it at first because the ball would
    // exhibit weird "snapping" behavior.
    // position.y = bumperCollider.y - sprite.height();
  }

  Bricks::CheckCollisions(collider, direction, speed);

  sprite.dest.x = position.x;
  sprite.dest.y = position.y;
}

void Ball::draw(SDL_Renderer *renderer) const {
  sprite.draw(renderer);
  // TODO: relocate this to a dedicated collider class
  // SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  // SDL_RenderRect(renderer, &sprite.dest);
}

void Ball::reset() {
  SDL_FPoint initBallPosition{
      .x = 800.f / 2,
      .y = 300,
  };

  position = initBallPosition;

  direction.x = (SDL_randf() * 1.6f) - 0.8f;
  direction.y = -1.0f;
  float len = SDL_sqrtf(direction.x * direction.x + direction.y * direction.y);
  direction.x /= len;
  direction.y /= len;
  speed = 100;
}
} // namespace Game