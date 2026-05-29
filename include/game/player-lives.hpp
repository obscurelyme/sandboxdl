#pragma once

#include "platform/spritesheet.hpp"

namespace Game {
class PlayerLives {
public:
  PlayerLives() = default;
  explicit PlayerLives(Sprites::Sprite sprite);

  void handleEvent(const SDL_Event &event);
  void subtractLives();
  int getLives() const;
  void draw(SDL_Renderer *renderer);
  Sprites::Sprite heartSprite;

private:
  static constexpr int MAX_LIVES = 3;
  int currentLives = MAX_LIVES;
};
} // namespace Game