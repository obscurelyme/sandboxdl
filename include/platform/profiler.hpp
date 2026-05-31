// Profiler.hpp
#pragma once

#if defined(BREAKOUT_ENABLE_PROFILING)
#include <tracy/Tracy.hpp>
#define SDL_PROFILE_FRAME() FrameMark
#define SDL_PROFILE_ZONE(name) ZoneScopedN(name)
#else
#define SDL_PROFILE_FRAME() ((void)0)
#define SDL_PROFILE_ZONE(name) ((void)0)
#endif