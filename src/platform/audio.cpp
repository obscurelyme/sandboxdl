#include "platform/audio.hpp"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <filesystem>

namespace Audio {

Sound::Sound(const std::string &name) : audioStream(nullptr), name(name) {
  auto assets = std::filesystem::path(SDL_GetBasePath()) / "assets" / "audio" /
                std::string(name + ".wav");

  bool success =
      SDL_LoadWAV(assets.string().c_str(), &spec, &buffer, &bufferSize);
  if (!success) {
    SDL_LogError(0, "[Audio::Sound] %s", SDL_GetError());
    SDL_assert_always(false);
  }
}

Sound::~Sound() {
  if (audioStream) {
    SDL_DestroyAudioStream(audioStream);
  }
}

void Sound::play() {
  audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                          &spec, nullptr, nullptr);
  SDL_PutAudioStreamData(audioStream, buffer, bufferSize);
  SDL_FlushAudioStream(audioStream);
  SDL_ResumeAudioStreamDevice(audioStream);
}

} // namespace Audio