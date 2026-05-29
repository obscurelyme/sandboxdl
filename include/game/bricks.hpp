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
  static void CheckCollisions(const SDL_FRect &ballCollider,
                              SDL_FPoint &ballDirection, float &ballSpeed);
  static void ResetBrickCount();

private:
  Sprites::Sprite sprite;
  SDL_FPoint position;
  bool hidden;
  static constexpr int numBricks{102};
  static int remainingBricks;
  static std::array<Bricks, numBricks> bricks;
  static std::array<std::string, 6> rowColors;
};
} // namespace Game