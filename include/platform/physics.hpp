#pragma once

#include <SDL3/SDL_render.h>
#include <box2d/box2d.h>
#include <box2d/math_functions.h>

namespace Physics {
constexpr float PIXELS_PER_UNIT = 50.0f;
constexpr float PI = B2_PI;

inline float deg2rads(float degrees) { return degrees * (PI / 180); }

inline float rads2deg(float radians) { return radians * (180 / PI); }

inline b2Rot deg2rot(float degrees) { return b2MakeRot(deg2rads(degrees)); }

inline b2Rot rad2rot(float radians) { return b2MakeRot(radians); }

inline float toPixels(float units) { return units * PIXELS_PER_UNIT; }

inline float toUnits(float pixels) { return pixels / PIXELS_PER_UNIT; }

inline SDL_FRect toRenderRect(const b2Vec2 &center, float halfWidth,
                              float halfHeight) {
  const float width = toPixels(halfWidth * 2.0f);
  const float height = toPixels(halfHeight * 2.0f);

  return SDL_FRect{
      .x = toPixels(center.x) - (width * 0.5f),
      .y = toPixels(center.y) - (height * 0.5f),
      .w = width,
      .h = height,
  };
}

inline void drawCircle(SDL_Renderer *renderer, b2Vec2 center, float radius,
                       SDL_Color color, int segments = 32) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

  const float cx = toPixels(center.x);
  const float cy = toPixels(center.y);
  const float r = toPixels(radius);

  float prevX = cx + r;
  float prevY = cy;

  for (int i = 1; i <= segments; ++i) {
    const float angle =
        (2.0f * PI * static_cast<float>(i)) / static_cast<float>(segments);
    const float x = cx + SDL_cosf(angle) * r;
    const float y = cy + SDL_sinf(angle) * r;

    SDL_RenderLine(renderer, prevX, prevY, x, y);

    prevX = x;
    prevY = y;
  }
}

class World {
public:
  static void Create();
  static void Simulate(float fixedDeltaTime);
  static void Destroy();
  static b2WorldId Id();

  static constexpr float FIXED_TIME_STEP{1.0f / 60.f};
  static constexpr int SUB_STEP_COUNT{4};

private:
  static b2WorldId id;
};

class BoxCollider {
public:
  explicit BoxCollider(const SDL_FRect &dimensions);

  b2BodyId getId() const;
  SDL_FPoint getPosition() const;
  SDL_FPoint getRotation() const;

private:
  b2BodyId id;
};

class CircleCollider {
public:
  CircleCollider() = default;

  // TODO: make the circle collider
};
} // namespace Physics
