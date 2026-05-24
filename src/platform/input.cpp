#include "platform/input.hpp"
#include <SDL3/SDL_log.h>

namespace Input {
void Manager::Swap() {
  keyboardState.swap();
  mouseState.swap();
}

const KeyboardState &Manager::Keyboard() { return keyboardState; }
const MouseState &Manager::Mouse() { return mouseState; }

void Manager::HandleInputEvent(SDL_Renderer *renderer, const SDL_Event &event) {
  switch (event.type) {
  case SDL_EVENT_KEY_UP:
  case SDL_EVENT_KEY_DOWN:
    keyboardState.update(event.key);
    return;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP:
    mouseState.updateButton(event.button);
    return;
  case SDL_EVENT_MOUSE_MOTION:
    mouseState.updateCoords(renderer, event.motion);
    return;
  case SDL_EVENT_MOUSE_WHEEL:
    // TODO: capture these events...
    return;
  default:
    return;
  }
}

void Manager::Reset() {
  keyboardState.reset();
  mouseState.reset();
}
} // namespace Input