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
  b2World_Step(id, fixedDeltaTime, SUB_STEP_COUNT);
}

b2WorldId World::Id() { return id; }

void World::Destroy() { b2DestroyWorld(id); }

BoxCollider::BoxCollider(const SDL_FRect &dimensions) {}
} // namespace Physics
