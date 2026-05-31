#include "game/credits-scene.hpp"
#include "game/game-scene.hpp"
#include "game/main-menu-scene.hpp"
#include "logging/handler.hpp"
#include "platform/debug-gui.hpp"
#include "platform/events.hpp"
#include "platform/input.hpp"
#include "platform/scene.hpp"
#include "platform/spritesheet.hpp"
#include "platform/text.hpp"
#include "platform/ui.hpp"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

int main(void) {
  /* #region Init Logic */
  SDL_SetLogOutputFunction(Logging::Handler, nullptr);

  bool success;

#ifndef NDEBUG
  success = SDL_SetHint(SDL_HINT_RENDER_GPU_DEBUG, "1");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return 1;
  }
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_TRACE);
#else
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
#endif
  success = SDL_SetHint(SDL_HINT_RENDER_DRIVER, "gpu");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return 1;
  }

  SDL_LogInfo(0, "Starting application");

  success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return 1;
  }

  success = SDL_SetAppMetadata("SandboxDL", "0.0.1", "com.obscure.sandboxdl");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return 1;
  }

  success = UI::FontManager::Init();
  if (!success) {
    return 1;
  }
  /* #endregion */

  /* #region Window and Renderer Creation */
  SDL_Window *window;
  SDL_Renderer *renderer;

#ifndef NDEBUG
  int windowFlags = 0;
#else
  int windowFlags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN;
#endif

  success = SDL_CreateWindowAndRenderer("Breakout", 1920, 1080, windowFlags,
                                        &window, &renderer);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return 1;
  }

  SDL_SetRenderLogicalPresentation(renderer, 800, 450,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_GPUDevice *gpu = SDL_GetGPURendererDevice(renderer);
  auto gpuDriver = SDL_GetGPUDeviceDriver(gpu);
  if (!gpuDriver) {
    SDL_LogError(0, "%s", SDL_GetError());
  } else {
    SDL_LogInfo(0, "Renderer created using driver <%s>", gpuDriver);
  }
  /* #endregion */
  UI::FontManager::SetRenderer(renderer);
  UI::FontManager::LoadFont("Tiny5");
  Sprites::Manager::SetRenderer(renderer);
  Sprites::Manager::LoadSpriteSheet("breakout-spritesheet");

  auto *sheet = Sprites::Manager::GetSpriteSheet("breakout-spritesheet");
  SDL_Surface *cursorSurface = sheet->getSurface("cursor", 3.5f);
  SDL_Cursor *cursor = SDL_CreateColorCursor(cursorSurface, 4, 4);
  SDL_DestroySurface(cursorSurface);
  SDL_SetCursor(cursor);

  /* #region Scenes */
  Scene::Manager::registerScene(Scene::SceneId::MainMenu,
                                std::make_unique<Game::MainMenuScene>());
  Scene::Manager::registerScene(Scene::SceneId::Game,
                                std::make_unique<Game::GameScene>());
  Scene::Manager::registerScene(Scene::SceneId::Credits,
                                std::make_unique<Game::CreditsScene>());

  Scene::Manager::registerTransition(Events::USER_PLAY_GAME,
                                     Scene::SceneId::Game);
  Scene::Manager::registerTransition(Events::USER_SHOW_CREDITS,
                                     Scene::SceneId::Credits);
  Scene::Manager::registerTransition(Events::USER_HIDE_CREDITS,
                                     Scene::SceneId::MainMenu);
  Scene::Manager::registerTransition(Events::USER_QUIT_GAME,
                                     Scene::SceneId::MainMenu);
  Scene::Manager::registerTransition(Events::USER_RETRY_GAME,
                                     Scene::SceneId::Game);

  Scene::Manager::start(Scene::SceneId::MainMenu);
  /* #endregion */

#ifndef NDEBUG
  auto fpsCounter = std::make_unique<DebugGui::FPS>();
#endif

  uint64_t lastTick = SDL_GetTicks();
  UI::InputContext inputCtx;
  bool running = true;
  while (running) {
#ifndef NDEBUG
    fpsCounter->update();
#endif
    uint64_t now = SDL_GetTicks();
    float deltaTime =
        SDL_min((now - lastTick) / 1000.0f, 0.05f); // capped at 50ms
    lastTick = now;

    // Clear previous frame input
    Input::Manager::Swap();

    // Poll input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
        Input::Manager::Reset();
      }

      if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
        lastTick = SDL_GetTicks();
      }

      Input::Manager::HandleInputEvent(renderer, event);
      inputCtx = UI::SnapshotCtx();
      Scene::Manager::handleEvent(event);

      if (event.type == SDL_EVENT_QUIT || event.type == Events::USER_QUIT_APP) {
        running = false;
        break;
      }
    }

    if (!running) {
      break;
    }

    // Update Scene
    Scene::Manager::update(deltaTime, inputCtx);

    // Render frame
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Draw Scene
    Scene::Manager::draw(renderer);
#ifndef NDEBUG
    fpsCounter->draw(renderer);
#endif

    SDL_RenderPresent(renderer);
  }

  SDL_LogInfo(0, "Closing application");
#ifndef NDEBUG
  fpsCounter.reset();
#endif
  SDL_DestroyCursor(cursor);
  Scene::Manager::shutdown();
  Sprites::Manager::Clear();
  UI::FontManager::Quit();
  SDL_Quit();
  SDL_LogInfo(0, "Application closed!");

  return 0;
}