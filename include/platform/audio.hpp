#pragma once

#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_events.h>
#include <array>
#include <string>

namespace Audio {
class Sound {
public:
  Sound(const std::string &name);
  ~Sound();

  void play();

private:
  struct Voice {
    SDL_AudioStream *stream{nullptr};
  };

  std::string name;
  SDL_AudioSpec spec;
  Uint8 *buffer;
  Uint32 bufferSize;

  std::array<Voice, 8> voices{};
  int nextVoiceIndex{0};

  int findIdleVoice() const;
  void destroyVoices();
};

class Manager {
public:
  static void PlaySound(SDL_AudioStream *stream);
  static void HandleEvent(const SDL_Event &event);
  static void Update();
  static void Clear();

private:
  static constexpr int MAX_ONE_SHOT_SOUNDS = 128;
  static std::array<SDL_AudioStream *, MAX_ONE_SHOT_SOUNDS> audioStreams;
};
} // namespace Audio