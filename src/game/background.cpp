#include "game/background.hpp"

namespace Game {
Background::Background(Sprites::Sprite sprite) : backgroundBrick(sprite) {}

void Background::draw(SDL_Renderer *renderer) {
  float columns = (800 / backgroundBrick.width()) + 1;
  float rows = 450 / backgroundBrick.height();

  for (int i = 0; i < rows; i++) {
    float offsetX = i % 2 == 0 ? 0 : backgroundBrick.width() * 0.5f;

    for (int j = 0; j < columns; j++) {
      backgroundBrick.dest.x = ((float)j * backgroundBrick.width()) - offsetX;
      backgroundBrick.dest.y = (float)i * backgroundBrick.height();
      backgroundBrick.draw(renderer);
    }
  }
}
} // namespace Game