#pragma once

#include <SDL3/SDL_audio.h>
#include <string>

namespace Audio {
class Sound {
public:
  Sound(const std::string &name);
  ~Sound();

  void play();

private:
  SDL_AudioStream *audioStream;
  std::string name;
  SDL_AudioSpec spec;
  Uint8 *buffer;
  Uint32 bufferSize;
};

// TODO: Manager that holds pointers to all one-shot audio streams.
// In the update loop, for each audio stream that is completed playing
// remove it from the list and delete it from memory.
} // namespace Audio