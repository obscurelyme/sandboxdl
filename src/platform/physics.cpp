#include "platform/physics.hpp"

namespace Physics {
b2WorldId Physics::World::id{};

void World::Create() {
  b2WorldDef def = b2DefaultWorldDef();

  def.enableSleep = true;
  def.gravity = b2Vec2{.x = 0, .y = 10};

  id = b2CreateWorld(&def);
}

void World::Simulate(float fixedDeltaTime) {
  // TODO: all physics objects, capture their previous state
  b2World_Step(id, fixedDeltaTime, SUB_STEP_COUNT);
  // TODO: all physics objects, capture their current state
}

b2WorldId World::Id() { return id; }

void World::Destroy() { b2DestroyWorld(id); }

BoxCollider::BoxCollider(const SDL_FRect &dimensions) {}

CircleCollider::CircleCollider(const CircleColliderProps &props) {
  this->props = props;
  this->createCollider();
}

CircleCollider::CircleCollider(CircleCollider &&other) noexcept
    : props(other.props), id(other.id), circle(other.circle) {
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
