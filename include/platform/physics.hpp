#pragma once

#include "platform/math.hpp"
#include <SDL3/SDL_render.h>
#include <box2d/box2d.h>
#include <box2d/math_functions.h>
#include <memory>
#include <string>
#include <vector>

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

struct PhysicsContext {
  SDL_Renderer *renderer;
  float alpha;
};

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

enum class BodyType {
  Static,
  Dynamic,
  Kinematic,
};

inline b2BodyType toB2BodyType(BodyType type) {
  switch (type) {
  case BodyType::Dynamic:
    return b2_dynamicBody;
  case BodyType::Static:
    return b2_staticBody;
  case BodyType::Kinematic:
    return b2_kinematicBody;
  }
}

inline b2Vec2 toB2Vec2(Math::Vec2 vec2) {
  return b2Vec2{
      .x = vec2.x,
      .y = vec2.y,
  };
}

inline b2Vec2 toB2Vec2(SDL_FPoint vec2) {
  return b2Vec2{
      .x = toUnits(vec2.x),
      .y = toUnits(vec2.y),
  };
}

inline SDL_FPoint toPoint(b2Vec2 vec2) {
  return SDL_FPoint{
      .x = toPixels(vec2.x),
      .y = toPixels(vec2.y),
  };
}

class Collider {
public:
  Collider() = default;
  virtual ~Collider() = default;

  void capturePreviousState() {
    previousPosition = currentPosition;
    previousRotation = currentRotation;
  }

  void captureCurrentState() {
    currentPosition = b2Body_GetPosition(id);
    currentRotation = b2Body_GetRotation(id);
  }

protected:
  virtual void release() = 0;

  b2BodyId id = b2_nullBodyId;
  b2Vec2 previousPosition = b2Vec2_zero;
  b2Vec2 currentPosition = b2Vec2_zero;
  b2Rot previousRotation = b2Rot_identity;
  b2Rot currentRotation = b2Rot_identity;
};

class World {
public:
  static void Create();
  static void Simulate(float fixedDeltaTime);
  static void Destroy();
  static void SetDebugRenderer(SDL_Renderer *renderer);
  /**
   * Draws all physics objects in the world, this function is a no-op in
   * release builds.
   */
  static void DebugDraw(float alpha);
  static b2WorldId Id();
  static void AddCollider(std::shared_ptr<Collider> collider);
  static void RemoveCollider(std::shared_ptr<Collider> collider);

  static constexpr float FIXED_TIME_STEP{1.0f / 60.f};
  static constexpr int SUB_STEP_COUNT{4};

private:
  static b2WorldId id;
  static b2DebugDraw debugDraw;
  static std::vector<std::weak_ptr<Collider>> colliders;
};

class BoxCollider : public Collider {
public:
  explicit BoxCollider(const SDL_FRect &dimensions);

protected:
  void release() override {}

  b2BodyId getId() const;
  SDL_FPoint getPosition() const;
  SDL_FPoint getRotation() const;
};

struct ChainColliderProps {
  std::vector<SDL_FPoint> points;
};

class ChainCollider : public Collider {
public:
  explicit ChainCollider(const std::vector<SDL_FPoint> &points);
  ~ChainCollider();
  void debugDraw(SDL_Renderer *renderer) const;

protected:
  void release() override;

private:
  std::vector<b2Vec2> _points;
  std::vector<SDL_FPoint> _authorPoints;
};

struct CircleColliderProps {
  SDL_FPoint position;
  BodyType type;
  SDL_FPoint center;
  bool isBullet;
  float density;
  float radius;
  float bounce;
  float friction;
  float rollResistance;
  std::string name;
};

class CircleCollider : public Collider {
public:
  CircleCollider() = default;
  explicit CircleCollider(const CircleColliderProps &props);
  CircleCollider(const CircleCollider &) = delete;
  CircleCollider &operator=(const CircleCollider &) = delete;
  CircleCollider(CircleCollider &&other) noexcept;
  CircleCollider &operator=(CircleCollider &&other) noexcept;
  ~CircleCollider();

  void applyForce(Math::Vec2 force) {
    b2Body_ApplyForceToCenter(id, toB2Vec2(force), true);
  }

  void applyTorque(float torque) { b2Body_ApplyTorque(id, torque, true); }

  void applyImpulse(Math::Vec2 impulse) {
    b2Body_ApplyLinearImpulseToCenter(id, toB2Vec2(impulse), true);
  };

  Math::Vec2 getPosition() {
    return Math::Vec2{
        .x = currentPosition.x,
        .y = currentPosition.y,
    };
  }

  void setTransform(Math::Vec2 pos) {
    b2Body_SetTransform(id, toB2Vec2(pos), deg2rot(0));
  }

  void debugDraw(SDL_Renderer *renderer, float alpha) {
    b2Vec2 p = b2Lerp(previousPosition, currentPosition, alpha);
    drawCircle(renderer, p, circle.radius, debugDrawColor);
  }

private:
  void release() override;
  void createCollider();

  CircleColliderProps props{};
  b2Circle circle{};
  SDL_Color debugDrawColor{
      .r = 0,
      .g = 255,
      .b = 255,
      .a = 255,
  };
};
} // namespace Physics
