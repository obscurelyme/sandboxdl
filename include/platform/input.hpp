#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_render.h>
#include <array>

namespace Input {
// TODO: implement gamepad state, but not for this project...just eventually

class KeyboardState {
public:
  /**
   * Checks if a key is currently pressed down
   */
  bool isDown(SDL_Scancode key) const { return current[key]; }

  /**
   * Checks if a key was pressed in the current frame
   */
  bool wasPressed(SDL_Scancode key) const {
    return !previous[key] && current[key];
  }

  /**
   * Checks if a key was released in the current frame
   */
  bool wasReleased(SDL_Scancode key) const {
    return previous[key] && !current[key];
  }

  void update(const SDL_KeyboardEvent &e) {
    if (e.scancode >= 0 && e.scancode < SDL_SCANCODE_COUNT) {
      current[e.scancode] = e.type == SDL_EVENT_KEY_DOWN;
    }
  }

  void swap() { previous = current; }

  void reset() {
    current.fill(false);
    previous.fill(false);
  }

  bool isShiftModifierDown() const {
    return (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;
  };

  bool isCtrlModifierDown() const {
    return (SDL_GetModState() & SDL_KMOD_CTRL) != 0;
  };

  bool isAltModifierDown() const {
    return (SDL_GetModState() & SDL_KMOD_ALT) != 0;
  };

private:
  std::array<bool, SDL_SCANCODE_COUNT> current{};
  std::array<bool, SDL_SCANCODE_COUNT> previous{};
};

class MouseState {
public:
  bool isLeftButtonDown() const { return current[SDL_BUTTON_LEFT]; }
  bool isRightButtonDown() const { return current[SDL_BUTTON_RIGHT]; }

  bool isLeftButtonPressed() const {
    return current[SDL_BUTTON_LEFT] && !previous[SDL_BUTTON_LEFT];
  }
  bool isRightButtonPressed() const {
    return current[SDL_BUTTON_RIGHT] && !previous[SDL_BUTTON_RIGHT];
  }

  bool isLeftButtonReleased() const {
    return !current[SDL_BUTTON_LEFT] && previous[SDL_BUTTON_LEFT];
  }
  bool isRightButtonReleased() const {
    return !current[SDL_BUTTON_RIGHT] && previous[SDL_BUTTON_RIGHT];
  }

  void updateButton(const SDL_MouseButtonEvent &e) {
    if (e.button >= 1 && e.button <= SDL_BUTTON_X2) {
      current[e.button] = e.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
    }
  }

  void updateCoords(SDL_Renderer *renderer, const SDL_MouseMotionEvent &e) {
    windowCoords.x = e.x;
    windowCoords.y = e.y;
    SDL_RenderCoordinatesFromWindow(renderer, windowCoords.x, windowCoords.y,
                                    &renderCoords.x, &renderCoords.y);
  }

  void swap() { previous = current; }

  void reset() {
    current.fill(false);
    previous.fill(false);
  }

  SDL_FPoint mouseWindowPosition() const { return windowCoords; }

  SDL_FPoint mouseRenderPosition() const { return renderCoords; }

private:
  SDL_FPoint windowCoords{.x = 0, .y = 0};
  SDL_FPoint renderCoords{.x = 0, .y = 0};
  std::array<bool, SDL_BUTTON_X2 + 1> current{};
  std::array<bool, SDL_BUTTON_X2 + 1> previous{};
};

class Manager {
public:
  static void Swap();
  static void HandleInputEvent(SDL_Renderer *renderer, const SDL_Event &event);
  static void Reset();

  static const KeyboardState &Keyboard();
  static const MouseState &Mouse();

private:
  inline static KeyboardState keyboardState{};
  inline static MouseState mouseState{};
};
} // namespace Input