#include "game/player-lives.hpp"
#include "platform/events.hpp"

namespace Game {
PlayerLives::PlayerLives(Sprites::Sprite sprite) : heartSprite(sprite) {
  heartSprite.scaleX = 1.5f;
  heartSprite.scaleY = 1.5f;
  heartSprite.dest.y = 450 - heartSprite.height() - 10;
}

int PlayerLives::getLives() const { return currentLives; }

void PlayerLives::handleEvent(const SDL_Event &event) {
  if (event.type == Events::USER_PLAYER_LOST_LIFE) {
    subtractLives();
  }
}

void PlayerLives::subtractLives() {
  currentLives--;
  if (currentLives < 0) {
    Events::Emit(Events::USER_GAME_OVER, nullptr, nullptr);
  }
}

void PlayerLives::draw(SDL_Renderer *renderer) {
  const float PADDING = 4;

  for (int i = 0; i < currentLives; i++) {
    heartSprite.dest.x = 750 - (i * (heartSprite.width() + PADDING));
    heartSprite.draw(renderer);
  }
}
} // namespace Game