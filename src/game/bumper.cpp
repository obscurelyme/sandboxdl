#include "game/bumper.hpp"
#include "platform/input.hpp"
#include <SDL3/SDL_render.h>

namespace Game {
Bumper::Bumper(const Sprites::Sprite &sprite, SDL_FPoint initPosition)
    : sprite(sprite), position(initPosition) {
  bounds = {
      .x = 50,
      .y = 0, // NOTE: don't care about Y-axis
      .w = 750 - sprite.width(),
      .h = 0, // NOTE: don't care about Y-axis
  };
}

void Bumper::update(float deltaTime) {
  bool moveLeft = Input::Manager::Keyboard().isDown(SDL_SCANCODE_A) ||
                  Input::Manager::Keyboard().isDown(SDL_SCANCODE_LEFT);
  bool moveRight = Input::Manager::Keyboard().isDown(SDL_SCANCODE_D) ||
                   Input::Manager::Keyboard().isDown(SDL_SCANCODE_RIGHT);

  if (moveLeft) {
    position.x -= 10 * speed * deltaTime;
  }

  if (moveRight) {
    position.x += 10 * speed * deltaTime;
  }

  if (position.x <= bounds.x) {
    position.x = bounds.x;
  }

  if (position.x >= bounds.w) {
    position.x = bounds.w;
  }

  sprite.dest.x = position.x;
  sprite.dest.y = position.y;
}

void Bumper::draw(SDL_Renderer *renderer) const { sprite.draw(renderer); }

SDL_FRect Bumper::collider() const { return sprite.dest; }
} // namespace Game