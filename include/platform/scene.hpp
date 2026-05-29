#pragma once

#include "platform/ui.hpp"
#include <SDL3/SDL_events.h>
#include <optional>
#include <unordered_map>

namespace Scene {
class IScene {
public:
  virtual ~IScene() = default;
  virtual void onEnter() = 0;
  virtual void onExit() = 0;
  virtual void handleEvent(const SDL_Event &event) = 0;
  virtual void update(float deltaTime, const UI::InputContext &ctx) = 0;
  virtual void draw(SDL_Renderer *renderer) = 0;
};

enum SceneId { None, MainMenu, Game, Credits };

class Manager {
public:
  static void registerScene(SceneId sceneId, std::unique_ptr<IScene> scene);
  static void registerTransition(Uint32 sdlUserEvent, SceneId targetScene);
  static void start(SceneId scene);
  static void shutdown();
  static void handleEvent(const SDL_Event &event);
  static void update(float deltaTime, const UI::InputContext &ctx);
  static void draw(SDL_Renderer *renderer);

private:
  static void transitionTo(SceneId sceneId);

  static std::unordered_map<SceneId, std::unique_ptr<IScene>> scenes;
  static std::unordered_map<Uint32, SceneId> transitions;
  static IScene *currentScene;
  static SceneId currentSceneId;
  static std::optional<SceneId> pendingScene;
};
}; // namespace Scene