#include "platform/physics.hpp"
#include <SDL3/SDL_log.h>
#include <vector>

namespace Physics {
namespace {
constexpr bool kDebugDrawShapes = true;
constexpr bool kDebugDrawJoints = false;
constexpr bool kDebugDrawBounds = true;

// static PhysicsContext gCtx{
//     .renderer = nullptr,
//     .alpha = 0.0f,
// };

SDL_Color toSDLColor(b2HexColor color) {
  const uint32_t value = static_cast<uint32_t>(color);
  return SDL_Color{
      .r = static_cast<Uint8>((value >> 16) & 0xFF),
      .g = static_cast<Uint8>((value >> 8) & 0xFF),
      .b = static_cast<Uint8>(value & 0xFF),
      .a = 255,
  };
}

void DebugDrawCircle(b2Vec2 center, float radius, b2HexColor color,
                     void *context) {
  auto *renderer = static_cast<SDL_Renderer *>(context);
  if (!renderer) {
    return;
  }

  drawCircle(renderer, center, radius, toSDLColor(color));
}

void DebugDrawSolidCircle(b2Transform transform, float radius, b2HexColor color,
                          void *context) {
  auto *renderer = static_cast<SDL_Renderer *>(context);
  if (!renderer) {
    return;
  }

  drawCircle(renderer, transform.p, radius, toSDLColor(color));
}

void DebugDrawPolygon(const b2Vec2 *vertices, int vertexCount, b2HexColor color,
                      void *context) {
  auto *renderer = static_cast<SDL_Renderer *>(context);
  if (!renderer) {
    return;
  }

  std::vector<SDL_FPoint> points;
  for (int i = 0; i < vertexCount; i++) {
    auto *vertex = vertices + i;
    points.emplace_back(SDL_FPoint{
        .x = toPixels(vertex->x),
        .y = toPixels(vertex->y),
    });
  }

  if (vertexCount == 4) {
    // NOTE: this is a rect and should be drawn like one
    // top-left, top-right, bottom-right, bottom-left
    SDL_FRect rect{
        .x = points.at(0).x,
        .y = points.at(0).y,
        .w = points.at(2).x - points.at(0).x,
        .h = points.at(2).y - points.at(0).y,
    };
    SDL_Color prevCol;
    SDL_GetRenderDrawColor(renderer, &prevCol.r, &prevCol.g, &prevCol.b,
                           &prevCol.a);
    SDL_Color col = toSDLColor(color);
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    SDL_RenderRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, prevCol.r, prevCol.g, prevCol.b,
                           prevCol.a);

    return;
  }

  SDL_RenderPoints(renderer, points.data(), vertexCount);
}

#ifndef NDEBUG
void DebugDrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void *context) {
  auto *renderer = static_cast<SDL_Renderer *>(context);
  if (!renderer) {
    return;
  }

  SDL_Color prevCol;
  SDL_GetRenderDrawColor(renderer, &prevCol.r, &prevCol.g, &prevCol.b,
                         &prevCol.a);
  SDL_Color col = toSDLColor(color);
  SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
  SDL_FPoint point1 = toPoint(p1);
  SDL_FPoint point2 = toPoint(p2);
  SDL_RenderLine(renderer, point1.x, point1.y, point2.x, point2.y);
  SDL_SetRenderDrawColor(renderer, prevCol.r, prevCol.g, prevCol.b, prevCol.a);
}
#endif

void DebugDrawPoint(b2Vec2 p, float size, b2HexColor color, void *context) {
  auto *renderer = static_cast<SDL_Renderer *>(context);
  if (!renderer) {
    return;
  }

  SDL_Color prevCol;
  SDL_GetRenderDrawColor(renderer, &prevCol.r, &prevCol.g, &prevCol.b,
                         &prevCol.a);
  SDL_Color col = toSDLColor(color);
  SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);

  // Render a small filled marker so debug vertices are visible at any zoom.
  const float pixelSize = size > 2.0f ? size : 2.0f;
  const float px = toPixels(p.x);
  const float py = toPixels(p.y);
  SDL_FRect marker{
      .x = px - (pixelSize * 0.5f),
      .y = py - (pixelSize * 0.5f),
      .w = pixelSize,
      .h = pixelSize,
  };
  SDL_RenderFillRect(renderer, &marker);

  SDL_SetRenderDrawColor(renderer, prevCol.r, prevCol.g, prevCol.b, prevCol.a);
}
} // namespace

b2WorldId World::id{};
b2DebugDraw World::debugDraw{};
std::vector<std::weak_ptr<Collider>> World::colliders{};

void World::Create() {
  b2WorldDef def = b2DefaultWorldDef();

  def.enableSleep = true;
  def.gravity = b2Vec2{.x = 0, .y = 10};

  id = b2CreateWorld(&def);
}

void World::SetDebugRenderer(SDL_Renderer *renderer) {
#ifndef NDEBUG
  debugDraw = b2DefaultDebugDraw();
  debugDraw.context = renderer;
  debugDraw.DrawCircleFcn = DebugDrawCircle;
  debugDraw.DrawSolidCircleFcn = DebugDrawSolidCircle;
  debugDraw.DrawPolygonFcn = DebugDrawPolygon;
  debugDraw.DrawSegmentFcn = DebugDrawSegment;
  debugDraw.DrawPointFcn = DebugDrawPoint;
  debugDraw.drawShapes = kDebugDrawShapes;
  debugDraw.drawJoints = kDebugDrawJoints;
  debugDraw.drawBounds = kDebugDrawBounds;
#endif
}

void World::AddCollider(std::shared_ptr<Collider> collider) {
  colliders.push_back(collider);
}

void World::Simulate(float fixedDeltaTime) {
  for (auto it = colliders.begin(); it != colliders.end();) {
    if (auto collider = it->lock()) {
      collider->capturePreviousState();
      ++it;
    } else {
      it = colliders.erase(it);
    }
  }
  b2World_Step(id, fixedDeltaTime, SUB_STEP_COUNT);
  for (auto it = colliders.begin(); it != colliders.end();) {
    if (auto collider = it->lock()) {
      collider->captureCurrentState();
      ++it;
    } else {
      it = colliders.erase(it);
    }
  }
}

void World::DebugDraw(float alpha) {
#ifndef NDEBUG
  b2World_Draw(id, &debugDraw);

  // auto *renderer = static_cast<SDL_Renderer *>(debugDraw.context);
  // if (renderer) {
  //   for (const auto &weakCollider : colliders) {
  //     if (auto collider = weakCollider.lock()) {
  //       if (auto *chain = dynamic_cast<ChainCollider *>(collider.get())) {
  //         chain->debugDraw(renderer);
  //       }
  //     }
  //   }
  // }
#endif
}

b2WorldId World::Id() { return id; }

void World::Destroy() {
  colliders.clear();
  b2DestroyWorld(id);
}

BoxCollider::BoxCollider(const SDL_FRect &dimensions) {}

ChainCollider::ChainCollider(const std::vector<SDL_FPoint> &points)
    : _points({}), _authorPoints(points) {
  for (auto &pt : points) {
    _points.push_back(toB2Vec2(pt));
  }

  b2SurfaceMaterial mat = b2DefaultSurfaceMaterial();
  mat.friction = 0.25;
  mat.restitution = 0;
  mat.rollingResistance = .5;
  mat.tangentSpeed = 0;

  b2ChainDef def = b2DefaultChainDef();
  def.points = _points.data();
  def.count = static_cast<int>(_points.size());
  def.materials = &mat;
  def.materialCount = 1;
  def.isLoop = false;

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_staticBody;
  bodyDef.position = {0, 0};
  id = b2CreateBody(World::Id(), &bodyDef);

  b2CreateChain(id, &def);
}

ChainCollider::~ChainCollider() { release(); }

void ChainCollider::debugDraw(SDL_Renderer *renderer) const {
  if (!renderer || _authorPoints.empty()) {
    return;
  }

  SDL_Color prevCol;
  SDL_GetRenderDrawColor(renderer, &prevCol.r, &prevCol.g, &prevCol.b,
                         &prevCol.a);

  // Draw authored chain points/segments in cyan to distinguish from Box2D
  // collision debug output.
  SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

  if (_authorPoints.size() >= 2) {
    SDL_RenderLines(renderer, _authorPoints.data(),
                    static_cast<int>(_authorPoints.size()));
  }

  constexpr float markerSize = 6.0f;
  const float halfSize = markerSize * 0.5f;
  for (const auto &point : _authorPoints) {
    SDL_FRect marker{
        .x = point.x - halfSize,
        .y = point.y - halfSize,
        .w = markerSize,
        .h = markerSize,
    };
    SDL_RenderFillRect(renderer, &marker);
  }

  SDL_SetRenderDrawColor(renderer, prevCol.r, prevCol.g, prevCol.b, prevCol.a);
}

void ChainCollider::release() {
  if (!B2_IS_NULL(id) && b2Body_IsValid(id)) {
    b2DestroyBody(id);
  }
  id = b2_nullBodyId;
}

CircleCollider::CircleCollider(const CircleColliderProps &props) : Collider() {
  this->props = props;
  this->createCollider();
}

CircleCollider::CircleCollider(CircleCollider &&other) noexcept
    : Collider(), props(other.props), circle(other.circle) {
  id = other.id;
  other.id = b2_nullBodyId;
}

CircleCollider &CircleCollider::operator=(CircleCollider &&other) noexcept {
  if (this != &other) {
    release();
    props = other.props;
    id = other.id;
    circle = other.circle;
    other.id = b2_nullBodyId;
  }
  return *this;
}

void CircleCollider::release() {
  if (!B2_IS_NULL(id) && b2Body_IsValid(id)) {
    b2DestroyBody(id);
  }
  id = b2_nullBodyId;
}

CircleCollider::~CircleCollider() { release(); }

void CircleCollider::createCollider() {
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = toB2BodyType(props.type);
  bodyDef.position = b2Vec2{
      .x = toUnits(props.position.x),
      .y = toUnits(props.position.y),
  };
  bodyDef.isBullet = props.isBullet;
  bodyDef.name = props.name.c_str();
  id = b2CreateBody(World::Id(), &bodyDef);

  circle.center = b2Vec2{
      .x = toUnits(props.center.x),
      .y = toUnits(props.center.y),
  };
  circle.radius = toUnits(props.radius);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = props.density;
  shapeDef.material.friction = props.friction;
  shapeDef.material.restitution = props.bounce;
  shapeDef.material.rollingResistance = props.rollResistance;
  b2CreateCircleShape(id, &shapeDef, &circle);
}
} // namespace Physics
