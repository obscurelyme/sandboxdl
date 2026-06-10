#include "platform/physics.hpp"
#include "platform/profiler.hpp"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>
#include <vector>

namespace Physics {
namespace {
constexpr bool kDebugDrawShapes = true;
constexpr bool kDebugDrawJoints = false;
constexpr bool kDebugDrawBounds = false;

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

void DrawCenterMarker(SDL_Renderer *renderer, b2Vec2 center,
                      SDL_Color markerColor) {
  const float cx = toPixels(center.x);
  const float cy = toPixels(center.y);
  constexpr float halfSize = 6.0f;

  SDL_Color previousColor;
  SDL_GetRenderDrawColor(renderer, &previousColor.r, &previousColor.g,
                         &previousColor.b, &previousColor.a);

  SDL_SetRenderDrawColor(renderer, markerColor.r, markerColor.g, markerColor.b,
                         markerColor.a);
  SDL_RenderLine(renderer, cx - halfSize, cy, cx + halfSize, cy);
  SDL_RenderLine(renderer, cx, cy - halfSize, cx, cy + halfSize);

  SDL_SetRenderDrawColor(renderer, previousColor.r, previousColor.g,
                         previousColor.b, previousColor.a);
}

void DebugDrawCircle(b2Vec2 center, float radius, b2HexColor color,
                     void *context) {
  auto *renderer = static_cast<SDL_Renderer *>(context);
  if (!renderer) {
    return;
  }

  drawCircle(renderer, center, radius, toSDLColor(color));
  DrawCenterMarker(renderer, center,
                   SDL_Color{.r = 255, .g = 255, .b = 0, .a = 255});
}

void DebugDrawSolidCircle(b2Transform transform, float radius, b2HexColor color,
                          void *context) {
  auto *renderer = static_cast<SDL_Renderer *>(context);
  if (!renderer) {
    return;
  }

  drawCircle(renderer, transform.p, radius, toSDLColor(color));
  DrawCenterMarker(renderer, transform.p,
                   SDL_Color{.r = 255, .g = 255, .b = 0, .a = 255});
}

void DebugDrawPolygon(const b2Vec2 *vertices, int vertexCount, b2HexColor color,
                      void *context) {
  auto *renderer = static_cast<SDL_Renderer *>(context);
  if (!renderer || !vertices || vertexCount < 2) {
    return;
  }

  std::vector<SDL_FPoint> points;
  points.reserve(static_cast<size_t>(vertexCount) + 1);

  for (int i = 0; i < vertexCount; i++) {
    auto *vertex = vertices + i;
    points.emplace_back(SDL_FPoint{
        .x = toPixels(vertex->x),
        .y = toPixels(vertex->y),
    });
  }

  // Close the loop by repeating the first point at the end.
  points.push_back(points.front());

  SDL_Color prevCol;
  SDL_GetRenderDrawColor(renderer, &prevCol.r, &prevCol.g, &prevCol.b,
                         &prevCol.a);
  SDL_Color col = toSDLColor(color);
  SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);

  SDL_RenderLines(renderer, points.data(), static_cast<int>(points.size()));

  SDL_SetRenderDrawColor(renderer, prevCol.r, prevCol.g, prevCol.b, prevCol.a);
}

void DebugDrawSolidPolygon(b2Transform transform, const b2Vec2 *vertices,
                           int vertexCount, float radius, b2HexColor color,
                           void *context) {
  (void)radius;
  auto *renderer = static_cast<SDL_Renderer *>(context);
  if (!renderer || !vertices || vertexCount < 2) {
    return;
  }

  std::vector<b2Vec2> worldVertices;
  worldVertices.reserve(static_cast<size_t>(vertexCount));
  for (int i = 0; i < vertexCount; ++i) {
    worldVertices.push_back(b2TransformPoint(transform, vertices[i]));
  }

  DebugDrawPolygon(worldVertices.data(), vertexCount, color, context);

  // Draw the body origin (transform.p) so authored local vertices can be
  // compared against the physics origin.
  DrawCenterMarker(renderer, transform.p,
                   SDL_Color{.r = 255, .g = 255, .b = 0, .a = 255});
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
  debugDraw.DrawSolidPolygonFcn = DebugDrawSolidPolygon;
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
#endif
}

b2WorldId World::Id() { return id; }

void World::Destroy() {
  colliders.clear();
  b2DestroyWorld(id);
}

PolygonCollider::PolygonCollider(const ColliderMeta &meta, SDL_FPoint pos)
    : Collider() {
  createPolygon(meta, pos, toB2BodyType(BodyType::Static), 1.0f, false, false);
}

PolygonCollider::PolygonCollider(const ColliderMeta &meta, SDL_FPoint pos,
                                 BodyType type, float scale) {
  createPolygon(meta, pos, toB2BodyType(type), scale, false, false);
}

PolygonCollider::PolygonCollider(const ColliderMeta &meta, SDL_FPoint pos,
                                 BodyType type, float scale, bool invertY,
                                 bool invertX)
    : Collider() {
  createPolygon(meta, pos, toB2BodyType(type), scale, invertX, invertY);
}

PolygonCollider::~PolygonCollider() { release(); }

void PolygonCollider::createPolygon(const ColliderMeta &meta, SDL_FPoint pos,
                                    b2BodyType type, float scale, bool invertX,
                                    bool invertY) {
  SDL_PROFILE_ZONE("Physics::PolygonCollider::Constructor");
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.name = meta.name.c_str();
  bodyDef.userData = this;
  bodyDef.type = type;
  bodyDef.position = toB2Vec2(pos);
  id = b2CreateBody(World::Id(), &bodyDef);

  for (auto &polygonMeta : meta.polygons) {
    std::vector<b2Vec2> scaledVertices{};
    scaledVertices.reserve(polygonMeta.vertices.size());

    for (auto &vert : polygonMeta.vertices) {
      scaledVertices.emplace_back(b2Vec2{
          .x = invertX ? -1 * vert.x * scale : vert.x * scale,
          .y = invertY ? -1 * vert.y * scale : vert.y * scale,
      });
    }

    b2Hull hull = b2ComputeHull(scaledVertices.data(), scaledVertices.size());
    SDL_assert(hull.count > 0);
    b2Polygon polygon = b2MakePolygon(&hull, 0);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = meta.density;
    shapeDef.material.friction = meta.friction;
    shapeDef.material.restitution = meta.restitution;
    shapeDef.material.rollingResistance = .1;
    b2CreatePolygonShape(id, &shapeDef, &polygon);
  }
}

void PolygonCollider::release() {
  if (!B2_IS_NULL(id) && b2Body_IsValid(id)) {
    b2DestroyBody(id);
  }
  id = b2_nullBodyId;
}

BoxCollider::BoxCollider(const SDL_FRect &dimensions) : Collider() {}

ChainCollider::ChainCollider(const std::vector<SDL_FPoint> &points)
    : Collider(), _points({}), _authorPoints(points) {
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
