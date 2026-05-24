#include "game/bricks.hpp"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

namespace Game {
std::array<std::string, 6> Bricks::rowColors{"bronze-pill", "green-pill",
                                             "red-pill",    "gold-pill",
                                             "purple-pill", "blue-pill"};
std::array<Bricks, 102> Bricks::bricks{};

Bricks::Bricks() : hidden(false), position({.x = 0, .y = 0}) {}

void Bricks::draw(SDL_Renderer *renderer) const {
  sprite.draw(renderer);
  // TODO: relocate this to a dedicated collider class
  // SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  // SDL_RenderRect(renderer, &sprite.dest);
}

void Bricks::Create(const Sprites::SpriteSheet *sheet) {
  const int MAX_BRICKS_COLUMNS = 17;
  const int MAX_BRICKS_ROWS = 6;
  const int MARGIN = 50;
  const int Y_MARGIN = 30;
  const float Y_PADDING = 10;
  const float PADDING = 9.1675;
  const float BRICK_HEIGHT = 16;
  const float BRICK_WIDTH = 32;

  for (int j = 0; j < MAX_BRICKS_ROWS; j++) {
    for (int i = 0; i < MAX_BRICKS_COLUMNS; i++) {
      Bricks newBrick{};

      newBrick.position = SDL_FPoint{
          .x = (float)(MARGIN + i * (BRICK_WIDTH + PADDING)),
          .y = (float)(Y_MARGIN + j * (BRICK_HEIGHT + Y_PADDING)),
      };

      SDL_LogInfo(0, "Brick position (%f, %f)", newBrick.position.x,
                  newBrick.position.y);

      // NOTE: change color based on row
      newBrick.sprite = sheet->getSprite(rowColors[j]);

      newBrick.sprite.dest.x = newBrick.position.x;
      newBrick.sprite.dest.y = newBrick.position.y;
      bricks[j * MAX_BRICKS_COLUMNS + i] = newBrick;
    }
  }
}

void Bricks::Draw(SDL_Renderer *renderer) {
  for (Bricks &b : bricks) {
    if (!b.hidden) {
      b.draw(renderer);
    }
  }
}

void Bricks::CheckCollisions(const SDL_FRect &ballCollider,
                             SDL_FPoint &ballDirection, float &ballSpeed) {
  for (Bricks &b : bricks) {
    if (!b.hidden &&
        SDL_HasRectIntersectionFloat(&ballCollider, &b.sprite.dest)) {
      b.hidden = true;
      ballDirection.y = -ballDirection.y;
      ballSpeed = SDL_min(400.0f, ballSpeed * 1.1f);
      break;
    }
  }
}
} // namespace Game