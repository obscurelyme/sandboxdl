#include "game/credits-scene.hpp"
#include "game/game-scene.hpp"
#include "game/main-menu-scene.hpp"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "logging/handler.hpp"
#include "platform/debug-gui.hpp"
#include "platform/events.hpp"
#include "platform/input.hpp"
#include "platform/physics.hpp"
#include "platform/profiler.hpp"
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
#include <imgui.h>

struct AppWindows {
  SDL_Window *gameWindow;
  SDL_Renderer *gameRenderer;
  Uint32 gameWindowId;

  SDL_Window *editorWindow;
  SDL_Renderer *editorRenderer;
  Uint32 editorWindowId;
};

bool InitAppWindows(AppWindows &app) {
#ifndef NDEBUG
  int windowFlags = 0;
#else
  int windowFlags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN;
#endif

  bool success =
      SDL_CreateWindowAndRenderer("Pinball Space Wars", 1920, 1080, windowFlags,
                                  &app.gameWindow, &app.gameRenderer);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return false;
  }

  SDL_SetRenderLogicalPresentation(app.gameRenderer, 800, 450,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_GPUDevice *gpu = SDL_GetGPURendererDevice(app.gameRenderer);
  auto gpuDriver = SDL_GetGPUDeviceDriver(gpu);
  if (!gpuDriver) {
    SDL_LogError(0, "%s", SDL_GetError());
  } else {
    SDL_LogInfo(0, "Renderer created using driver <%s>", gpuDriver);
    SDL_SetGPUSwapchainParameters(gpu, app.gameWindow,
                                  SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                  SDL_GPU_PRESENTMODE_IMMEDIATE);
  }

  app.gameWindowId = SDL_GetWindowID(app.gameWindow);

  success =
      SDL_CreateWindowAndRenderer("Editor", 1920, 1080, SDL_WINDOW_RESIZABLE,
                                  &app.editorWindow, &app.editorRenderer);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return false;
  }

  app.editorWindowId = SDL_GetWindowID(app.editorWindow);

  return true;
}

void DestroyAppWindows(AppWindows &app) {
  SDL_DestroyRenderer(app.gameRenderer);
  SDL_DestroyWindow(app.gameWindow);

  SDL_DestroyRenderer(app.editorRenderer);
  SDL_DestroyWindow(app.editorWindow);
}

bool InitHints() {
  SDL_SetLogOutputFunction(Logging::Handler, nullptr);

  bool success;

#ifndef NDEBUG
  success = SDL_SetHint(SDL_HINT_RENDER_GPU_DEBUG, "1");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return false;
  }
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_TRACE);
#else
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
#endif
  success = SDL_SetHint(SDL_HINT_RENDER_DRIVER, "gpu");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return false;
  }

  return true;
}

bool InitApp() {
  SDL_LogInfo(0, "Starting application");

  bool success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return false;
  }

  success = SDL_SetAppMetadata("SandboxDL", "0.0.1", "com.obscure.sandboxdl");
  if (!success) {
    SDL_LogError(0, "%s", SDL_GetError());
    return false;
  }

  success = UI::FontManager::Init();
  if (!success) {
    return false;
  }

  return true;
}

void InitEditor(const AppWindows &app) {
  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  if (main_scale < 1.0f) {
    main_scale = 1.0f;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  io.FontGlobalScale = main_scale;

  ImGui::StyleColorsDark();

  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
  style.AntiAliasedFill = true;
  style.AntiAliasedLines = true;

  ImGui_ImplSDL3_InitForSDLRenderer(app.editorWindow, app.editorRenderer);
  ImGui_ImplSDLRenderer3_Init(app.editorRenderer);
}

Uint32 GetWindowId(const SDL_Event &e, const AppWindows &app) {
  switch (e.type) {
  case SDL_EVENT_WINDOW_FOCUS_LOST:
  case SDL_EVENT_WINDOW_FOCUS_GAINED:
  case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    return e.window.windowID;
  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP:
    return e.key.windowID;
  case SDL_EVENT_MOUSE_MOTION:
    return e.motion.windowID;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP:
    return e.button.windowID;
  case SDL_EVENT_MOUSE_WHEEL:
    return e.wheel.windowID;
  default:
    return Events::IsUserEvent(e.type) ? app.gameWindowId : 0;
  }
}

int main(void) {
  if (!InitHints()) {
    SDL_LogError(0, "[App] Unable to set SDL hints. Program will exit.");
    return 1;
  }

  if (!InitApp()) {
    SDL_LogError(
        0, "[App] Unable to initialize SDL system[s]. Program will exit.");
    return 1;
  }

  AppWindows app{};
  if (!InitAppWindows(app)) {
    SDL_LogError(
        0, "[App] Unable to create application window[s]. Program will exit.");
    return 1;
  }

  InitEditor(app);

  Physics::World::SetDebugRenderer(app.gameRenderer);

  UI::FontManager::SetRenderer(app.gameRenderer);
  UI::FontManager::LoadFont("Tiny5");
  Sprites::Manager::SetRenderer(app.gameRenderer);
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

  Scene::Manager::start(Scene::SceneId::Game);
  /* #endregion */

#ifndef NDEBUG
  auto fpsCounter = std::make_unique<DebugGui::FPS>();
#endif

  const Uint64 MAX_STEPS_PER_FRAME = 5;
  const float FIXED_DELTA_TIME = Physics::World::FIXED_TIME_STEP;
  Uint64 lastTick = SDL_GetTicks();
  Uint64 steps = 0;
  Uint64 now = 0;
  float accumulator = 0.0f;
  float alpha = 0.0f;
  UI::InputContext inputCtx;
  bool running = true;
  bool demo = true;

  while (running) {
    SDL_PROFILE_FRAME();
#ifndef NDEBUG
    fpsCounter->update();
#endif
    now = SDL_GetTicks();
    float deltaTime = SDL_clamp((now - lastTick) / 1000.0f, 0.0f,
                                0.05f); // capped from 0ms to 50ms
    lastTick = now;

    // Clear previous frame input
    {
      SDL_PROFILE_ZONE("Input Swap");
      Input::Manager::Swap();
    }

    // Poll input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      const bool closeRequested =
          event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          (event.window.windowID == app.gameWindowId ||
           event.window.windowID == app.editorWindowId);
      const bool quitEvent =
          event.type == SDL_EVENT_QUIT || event.type == Events::USER_QUIT_APP;

      if (closeRequested || quitEvent) {
        running = false;
        break;
      }

      const Uint32 windowId = GetWindowId(event, app);

      // Editor Events
      if (windowId == app.editorWindowId) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        // TODO: Editor::HandleEvent(event);
        continue;
      }

      // Game Events
      if (windowId == app.gameWindowId) {
        if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
          Input::Manager::Reset();
        }

        if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
          lastTick = SDL_GetTicks();
        }

        {
          SDL_PROFILE_ZONE("Handle Input Event");
          Input::Manager::HandleInputEvent(app.gameRenderer, event);
          inputCtx = UI::SnapshotCtx();
          Scene::Manager::handleEvent(event);
        }
      }
    }

    if (!running) {
      break;
    }

    // Update Scene
    {
      SDL_PROFILE_ZONE("Scene Update");
      Scene::Manager::update(deltaTime, inputCtx);
    }

    // Fixed Update Scene
    {
      SDL_PROFILE_ZONE("Scene Fixed Update");
      accumulator += deltaTime;
      steps = 0;
      while (accumulator >= FIXED_DELTA_TIME && steps < MAX_STEPS_PER_FRAME) {
        Scene::Manager::fixedUpdate(FIXED_DELTA_TIME);
        accumulator -= FIXED_DELTA_TIME;
        steps++;
      }
      // NOTE: anti-spiral fallback:
      if (steps == MAX_STEPS_PER_FRAME) {
        accumulator = 0.0f;
      }

      // NOTE: alpha for render interpolation + clamp to prevent drift
      alpha = SDL_clamp(accumulator / FIXED_DELTA_TIME, 0.0f, 1.0f);
    }

    // Render frame
    {
      SDL_PROFILE_ZONE("Game Render");
      SDL_SetRenderDrawColor(app.gameRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
      SDL_RenderClear(app.gameRenderer);

      // Draw Scene
      Scene::Manager::draw(app.gameRenderer, alpha);

#ifndef NDEBUG
      fpsCounter->draw(app.gameRenderer);
#endif

      SDL_RenderPresent(app.gameRenderer);
    }

    // Editor Render
    {
      SDL_PROFILE_ZONE("Editor Render");
      SDL_SetRenderDrawColor(app.editorRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
      SDL_RenderClear(app.editorRenderer);

      ImGui_ImplSDLRenderer3_NewFrame();
      ImGui_ImplSDL3_NewFrame();
      ImGui::NewFrame();

      // TODO: Draw Editor Panels...
      ImGui::ShowDemoWindow(&demo);

      ImGui::Render();
      ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                            app.editorRenderer);
      SDL_RenderPresent(app.editorRenderer);
    }
  }

  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_LogInfo(0, "Closing application");
#ifndef NDEBUG
  fpsCounter.reset();
#endif
  SDL_DestroyCursor(cursor);
  Scene::Manager::shutdown();
  Sprites::Manager::Clear();
  UI::FontManager::Quit();
  DestroyAppWindows(app);
  SDL_Quit();
  SDL_LogInfo(0, "Application closed!");

  return 0;
}
