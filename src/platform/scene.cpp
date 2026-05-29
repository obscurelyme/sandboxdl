#include "platform/scene.hpp"
#include "platform/events.hpp"

namespace Scene {
std::unordered_map<SceneId, std::unique_ptr<IScene>> Manager::scenes{};
std::unordered_map<Uint32, SceneId> Manager::transitions{};
IScene *Manager::currentScene = nullptr;
SceneId Manager::currentSceneId = SceneId::None;
std::optional<SceneId> Manager::pendingScene{};

void Manager::registerScene(SceneId sceneId, std::unique_ptr<IScene> scene) {
  auto iterator = scenes.find(sceneId);
  if (iterator != scenes.end()) {
    SDL_LogWarn(
        0,
        "[Scene::Manager] Invalid attempt to re-register existing scene '%d'",
        sceneId);
    return;
  }

  scenes[sceneId] = std::move(scene);
}

void Manager::registerTransition(Uint32 sdlUserEvent, SceneId targetScene) {
  auto iterator = transitions.find(sdlUserEvent);
  if (iterator != transitions.end()) {
    SDL_LogWarn(0,
                "[Scene::Manager] Invalid attempt to re-register existing "
                "transition '%d'",
                sdlUserEvent);
    return;
  }

  transitions[sdlUserEvent] = targetScene;
}

void Manager::start(SceneId scene) {
  currentSceneId = scene;
  currentScene = scenes.at(scene).get();
  currentScene->onEnter();
}

void Manager::shutdown() {
  if (currentScene != nullptr) {
    currentScene->onExit();
  }

  currentScene = nullptr;
  currentSceneId = SceneId::None;
  pendingScene = {};

  scenes.clear();
  transitions.clear();
}

void Manager::handleEvent(const SDL_Event &event) {
  if (event.type == Events::USER_QUIT_APP) {
    shutdown();
    return;
  }

  auto iterator = transitions.find(event.type);
  if (iterator != transitions.end()) {
    pendingScene = iterator->second;
  }
  currentScene->handleEvent(event);
}

void Manager::update(float deltaTime, const UI::InputContext &ctx) {
  if (pendingScene.has_value()) {
    transitionTo(pendingScene.value());
  }

  currentScene->update(deltaTime, ctx);
}

void Manager::draw(SDL_Renderer *renderer) { currentScene->draw(renderer); }

void Manager::transitionTo(SceneId sceneId) {
  currentScene->onExit();
  currentScene = scenes.at(sceneId).get();
  currentScene->onEnter();
  pendingScene = {};
  currentSceneId = sceneId;
}
} // namespace Scene