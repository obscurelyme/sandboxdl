#pragma once

#include <box2d/box2d.h>
#include <string>
#include <tinyxml2.h>
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

class XmlColliderParser {
public:
  explicit XmlColliderParser(const std::string &fileName);

private:
  std::string loadFullFilePath(const std::string &fileName);
  void loadDocument(const std::string &filePath);
  b2Vec2 loadAnchor(const std::string &anchorStr);
  void loadFixtures(ColliderMeta &meta, tinyxml2::XMLElement *fixture);
  void loadPolygons(ColliderMeta &meta, tinyxml2::XMLElement *polygon);

  std::vector<ColliderMeta> colliderMetas;
};
} // namespace Physics
