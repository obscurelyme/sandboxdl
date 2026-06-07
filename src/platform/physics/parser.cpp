#include "platform/physics/parser.hpp"
#include "platform/profiler.hpp"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <cctype>
#include <filesystem>
#include <tinyxml2.h>

namespace Physics {

namespace {
std::vector<std::string> splitString(const std::string &str, char delimiter) {
  std::vector<std::string> strings;
  std::string token;

  for (auto c : str) {
    if (c != delimiter) {
      token.push_back(c);
    } else {
      strings.push_back(token);
      token.clear();
    }
  }

  strings.push_back(token);

  return strings;
}

std::string trimString(const std::string &str) {
  std::string out;

  for (auto c : str) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      out.push_back(c);
    }
  }

  return out;
}
} // namespace

XmlColliderParser::XmlColliderParser(const std::string &fileName)
    : colliderMetas({}) {
  SDL_PROFILE_ZONE("XmlColliderParser::Constructor");
  SDL_LogInfo(0, "[Physics::XmlColliderParser] Loading xml file '%s'",
              fileName.c_str());

  std::string assetPath = loadFullFilePath(fileName);
  loadDocument(assetPath);
}

std::string XmlColliderParser::loadFullFilePath(const std::string &fileName) {
  SDL_PROFILE_ZONE("XmlColliderParser::Constructor::loadFullFilePath");
  auto filePathWithExt = fileName + ".xml";
  auto assetPath = std::filesystem::path(SDL_GetBasePath()) / "assets" /
                   "colliders" / filePathWithExt;

  return assetPath.string();
}

void XmlColliderParser::loadDocument(const std::string &filePath) {
  SDL_PROFILE_ZONE("XmlColliderParser::Constructor::loadDocument");
  tinyxml2::XMLDocument doc;
  auto result = doc.LoadFile(filePath.c_str());
  if (result != tinyxml2::XMLError::XML_SUCCESS) {
    SDL_LogError(0, "[Physics::XmlColliderParser] '%s' could not be loaded",
                 filePath.c_str());
    SDL_assert_always(false);
  }

  auto rootEl = doc.RootElement()->FirstChildElement();
  auto bodyEl = rootEl->FirstChildElement();

  do {
    ColliderMeta meta{};

    meta.name = bodyEl->Attribute("name");
    auto *child = bodyEl->FirstChildElement();

    do {
      if (std::string(child->Name()) == "anchorpoint") {
        meta.anchor = loadAnchor(child->GetText());
      }

      if (std::string(child->Name()) == "fixtures") {
        loadFixtures(meta, child->FirstChildElement());
      }

      child = child->NextSiblingElement();
    } while (child);

    colliderMetas.push_back(meta);
    bodyEl = bodyEl->NextSiblingElement();
  } while (bodyEl);
}

b2Vec2 XmlColliderParser::loadAnchor(const std::string &anchorStr) {
  SDL_PROFILE_ZONE("XmlColliderParser::Constructor::loadAnchor");
  auto anchorVec = splitString(trimString(anchorStr), ',');
  SDL_assert(anchorVec.size() == 2);

  return {
      .x = std::stof(trimString(anchorVec.at(0))),
      .y = std::stof(trimString(anchorVec.at(1))),
  };
}

void XmlColliderParser::loadFixtures(ColliderMeta &meta,
                                     tinyxml2::XMLElement *el) {
  SDL_PROFILE_ZONE("XmlColliderParser::Constructor::loadFixtures");
  do {
    if (std::string(el->Name()) == "density") {
      meta.density = el->FloatText();
    } else if (std::string(el->Name()) == "friction") {
      meta.friction = el->FloatText();
    } else if (std::string(el->Name()) == "restitution") {
      meta.restitution = el->FloatText();
    } else if (std::string(el->Name()) == "polygons") {
      loadPolygons(meta, el->FirstChildElement());
    } else {
      SDL_LogTrace(0,
                   "[Physics::XmlColliderParser] Ignoring XML field '%s' from "
                   "'fixture' field",
                   el->Name());
    }

    el = el->NextSiblingElement();
  } while (el);
}

void XmlColliderParser::loadPolygons(ColliderMeta &meta,
                                     tinyxml2::XMLElement *el) {
  SDL_PROFILE_ZONE("XmlColliderParser::Constructor::loadPolygons");
  meta.polygons = {};

  do {
    if (std::string(el->Name()) == "polygon") {
      std::vector<std::string> p = splitString(trimString(el->GetText()), ',');
      // NOTE: assert that we have an even number of floats since
      // we need both (x, y) for each vertex
      SDL_assert(p.size() % 2 == 0);

      std::vector<b2Vec2> vertices{};
      for (int i = 0; i < p.size(); i += 2) {
        vertices.push_back({
            .x = std::stof(trimString(p.at(i))),
            .y = std::stof(trimString(p.at(i + 1))),
        });
      }

      meta.polygons.push_back({.vertices = vertices});
    }

    el = el->NextSiblingElement();
  } while (el);
}
} // namespace Physics
