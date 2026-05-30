#include "platform/audio.hpp"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <filesystem>

namespace Audio {

Sound::Sound(const std::string &name) : name(name) {
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
  SDL_free(buffer);
  SDL_LogDebug(0, "[Audio::Sound] Freeing audio buffer for sound '%s'",
               name.c_str());
}

void Sound::play() {
  SDL_AudioStream *audioStream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
  SDL_SetAudioStreamGain(audioStream, 0.25);
  SDL_PutAudioStreamData(audioStream, buffer, bufferSize);
  SDL_FlushAudioStream(audioStream);
  SDL_ResumeAudioStreamDevice(audioStream);
  Manager::PlaySound(audioStream);
}

std::array<SDL_AudioStream *, Manager::MAX_ONE_SHOT_SOUNDS>
    Manager::audioStreams{};

void Manager::PlaySound(SDL_AudioStream *stream) {
  for (int i = 0; i < MAX_ONE_SHOT_SOUNDS; i++) {
    if (audioStreams[i] == nullptr) {
      audioStreams[i] = stream;
      return;
    }
  }
}

void Manager::Update() {
  for (int i = 0; i < MAX_ONE_SHOT_SOUNDS; i++) {
    if (audioStreams[i] != nullptr &
        SDL_GetAudioStreamQueued(audioStreams[i]) <= 0) {
      SDL_DestroyAudioStream(audioStreams[i]);
      audioStreams[i] = nullptr;
    }
  }
}

void Manager::Clear() {
  for (int i = 0; i < MAX_ONE_SHOT_SOUNDS; i++) {
    if (audioStreams[i] != nullptr) {
      SDL_DestroyAudioStream(audioStreams[i]);
      audioStreams[i] = nullptr;
    }
  }
}

} // namespace Audio