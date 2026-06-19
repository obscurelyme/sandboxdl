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
    return;
  }

  voices.fill(Voice{
      .stream = nullptr,
  });

  for (int i = 0; i < 8; i++) {
    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!stream) {
      SDL_LogError(0, "[Audio::Sound] Failed to open stream for '%s': '%s'",
                   name.c_str(), SDL_GetError());
      SDL_assert_always(false);
      return;
    }

    if (!SDL_SetAudioStreamGain(stream, 0.25f)) {
      SDL_LogWarn(0, "[Audio::Sound] Failed to set gain for '%s': %s",
                  name.c_str(), SDL_GetError());
    }

    if (!SDL_ResumeAudioStreamDevice(stream)) {
      SDL_LogError(0,
                   "[Audio::Sound] Failed to resume stream device for '%s': %s",
                   name.c_str(), SDL_GetError());
      SDL_assert_always(false);
    }

    voices[i].stream = stream;
  }
}

Sound::~Sound() {
  for (auto &voice : voices) {
    if (voice.stream) {
      SDL_DestroyAudioStream(voice.stream);
      voice.stream = nullptr;
    }
  }

  if (buffer) {
    SDL_free(buffer);
    buffer = nullptr;
  }

  SDL_LogDebug(0, "[Audio::Sound] Freed audio resources for sound '%s'",
               name.c_str());
}

void Sound::play() {
  int voiceIndex = nextVoiceIndex;
  nextVoiceIndex = (nextVoiceIndex + 1) % static_cast<int>(voices.size());

  SDL_AudioStream *stream = voices[voiceIndex].stream;

  if (!stream || !buffer || bufferSize == 0) {
    SDL_LogWarn(0, "[Audio::Sound] Cannot play '%s': stream/buffer invalid",
                name.c_str());
    return;
  }

  if (SDL_GetAudioStreamQueued(stream) > 0) {
    if (!SDL_ClearAudioStream(stream)) {
      SDL_LogWarn(0, "[Audio::Sound] Failed to clear stream for '%s': %s",
                  name.c_str(), SDL_GetError());
    }
  }

  if (!SDL_PutAudioStreamData(stream, buffer, bufferSize)) {
    SDL_LogError(0, "[Audio::Sound] Failed to queue '%s': '%s", name.c_str(),
                 SDL_GetError());
    return;
  }

  if (!SDL_FlushAudioStream(stream)) {
    SDL_LogWarn(0, "[Audio::Sound] Failed to flush '%s': '%s'", name.c_str(),
                SDL_GetError());
    return;
  }
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
    if (audioStreams[i] != nullptr &&
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