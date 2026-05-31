#include "game/background.hpp"
#include "platform/profiler.hpp"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>

namespace Game {
namespace {
constexpr float kLogicalWidth = 800.0f;
constexpr float kLogicalHeight = 450.0f;
constexpr SDL_FColor kDefaultVertexColor{1.0f, 1.0f, 1.0f, 1.0f};
} // namespace

Background::Background(Sprites::Sprite sprite) : backgroundBrick(sprite) {
  SDL_PROFILE_ZONE("Game::Background::Contructor");
  buildBatch();
}

void Background::buildBatch() {
  SDL_PROFILE_ZONE("Game::Background::buildBatch");
  const float brickWidth = backgroundBrick.width();
  const float brickHeight = backgroundBrick.height();
  if (brickWidth <= 0.0f || brickHeight <= 0.0f) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Invalid background brick size (%f, %f)", brickWidth,
                 brickHeight);
    SDL_assert_always(false);
    return;
  }

  float textureHeight = 0.0f;
  float textureWidth = 0.0f;
  if (!SDL_GetTextureSize(backgroundBrick.texture(), &textureWidth,
                          &textureHeight)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to query background texture size: %s", SDL_GetError());
    SDL_assert_always(false);
    return;
  }

  const SDL_FRect &frame = backgroundBrick.source();
  const float u0 = frame.x / textureWidth;
  const float v0 = frame.y / textureHeight;
  const float u1 = (frame.x + frame.w) / textureWidth;
  const float v1 = (frame.y + frame.h) / textureHeight;
  const int columns = static_cast<int>(kLogicalWidth / brickWidth) + 1;
  const int rows = static_cast<int>(kLogicalHeight / brickHeight);
  const size_t tileCount = static_cast<size_t>(columns) * rows;

  batchedVertices.clear();
  batchedIndices.clear();
  batchedVertices.reserve(tileCount * 4);
  batchedIndices.reserve(tileCount * 6);

  for (int row = 0; row < rows; row++) {
    const float offsetX = row % 2 == 0 ? 0.0f : brickWidth * 0.5f;
    const float y = static_cast<float>(row) * brickHeight;

    for (int col = 0; col < columns; col++) {
      const float x = (static_cast<float>(col) * brickWidth) - offsetX;
      const int baseVertex = static_cast<int>(batchedVertices.size());

      batchedVertices.push_back(
          SDL_Vertex{{x, y}, kDefaultVertexColor, {u0, v0}});
      batchedVertices.push_back(
          SDL_Vertex{{x + brickWidth, y}, kDefaultVertexColor, {u1, v0}});
      batchedVertices.push_back(SDL_Vertex{
          {x + brickWidth, y + brickHeight}, kDefaultVertexColor, {u1, v1}});
      batchedVertices.push_back(
          SDL_Vertex{{x, y + brickHeight}, kDefaultVertexColor, {u0, v1}});

      batchedIndices.push_back(baseVertex);
      batchedIndices.push_back(baseVertex + 1);
      batchedIndices.push_back(baseVertex + 2);
      batchedIndices.push_back(baseVertex);
      batchedIndices.push_back(baseVertex + 2);
      batchedIndices.push_back(baseVertex + 3);
    }
  }
}

void Background::draw(SDL_Renderer *renderer) {
  SDL_PROFILE_ZONE("Game::Background::draw");
  if (batchedVertices.empty() || batchedIndices.empty()) {
    buildBatch();
  }

  if (!SDL_RenderGeometry(
          renderer, backgroundBrick.texture(), batchedVertices.data(),
          static_cast<int>(batchedVertices.size()), batchedIndices.data(),
          static_cast<int>(batchedIndices.size()))) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to render batched background geometry: %s",
                 SDL_GetError());
    SDL_assert_always(false);
  }
}
} // namespace Game
