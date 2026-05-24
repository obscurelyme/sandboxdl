#include "game/bumper.hpp"
#include "platform/input.hpp"

namespace Game {
Bumper::Bumper(const Sprites::Sprite &sprite) : sprite(sprite) {}

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

  sprite.dest.x = position.x;
  sprite.dest.y = position.y;
}

void Bumper::draw(SDL_Renderer *renderer) const { sprite.draw(renderer); }
} // namespace Game