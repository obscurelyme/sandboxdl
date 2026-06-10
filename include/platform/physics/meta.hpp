#pragma once

#include <box2d/box2d.h>
#include <string>
#include <vector>

namespace Physics {

struct ColliderPolygon {
  std::vector<b2Vec2> vertices;
};

struct ColliderMeta {
  std::string name;
  b2Vec2 anchor;
  float density;
  float friction;
  float restitution;
  std::vector<ColliderPolygon> polygons;
};

} // namespace Physics
