#pragma once

#include "platform/physics/meta.hpp"
#include <box2d/box2d.h>
#include <string>
#include <tinyxml2.h>
#include <vector>

namespace Physics {
class XmlColliderParser {
public:
  explicit XmlColliderParser(const std::string &fileName);

  std::vector<ColliderMeta> colliderMetas;

private:
  std::string loadFullFilePath(const std::string &fileName);
  void loadDocument(const std::string &filePath);
  b2Vec2 loadAnchor(const std::string &anchorStr);
  void loadFixtures(ColliderMeta &meta, tinyxml2::XMLElement *fixture);
  void loadPolygons(ColliderMeta &meta, tinyxml2::XMLElement *polygon);
};
} // namespace Physics
