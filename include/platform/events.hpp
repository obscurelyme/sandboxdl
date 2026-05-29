#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>

namespace Events {
const inline Uint32 USER_PLAY_GAME{SDL_RegisterEvents(1)};
const inline Uint32 USER_GAME_OVER{SDL_RegisterEvents(1)};
const inline Uint32 USER_GAME_WIN{SDL_RegisterEvents(1)};
const inline Uint32 USER_RETRY_GAME{SDL_RegisterEvents(1)};
const inline Uint32 USER_QUIT_GAME{SDL_RegisterEvents(1)};
const inline Uint32 USER_QUIT_APP{SDL_RegisterEvents(1)};
const inline Uint32 USER_PAUSE{SDL_RegisterEvents(1)};
const inline Uint32 USER_UNPAUSE{SDL_RegisterEvents(1)};
const inline Uint32 USER_SHOW_CREDITS{SDL_RegisterEvents(1)};
const inline Uint32 USER_HIDE_CREDITS{SDL_RegisterEvents(1)};
const inline Uint32 USER_PLAYER_LOST_LIFE{SDL_RegisterEvents(1)};

inline void Emit(Uint32 event, void *data1, void *data2) {
  SDL_UserEvent userEvent{
      .type = event,
      .timestamp = SDL_GetTicksNS(),
      .code = -1,
      .data1 = data1,
      .data2 = data2,
  };
  SDL_Event eventToDispatch{.user = userEvent};

  SDL_PushEvent(&eventToDispatch);
}
} // namespace Events