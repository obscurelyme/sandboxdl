#pragma once

#include "platform/spritesheet.hpp"
#include <vector>

namespace Game {
class Background {
public:
  Background() = default;
  explicit Background(Sprites::Sprite backgroundBrick);
  void draw(SDL_Renderer *renderer);

  Sprites::Sprite backgroundBrick;

private:
  void buildBatch();
  std::vector<SDL_Vertex> batchedVertices;
  std::vector<int> batchedIndices;
};
} // namespace Game
