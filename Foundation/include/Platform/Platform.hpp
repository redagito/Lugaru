/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _PLATFORM_HPP_
#define _PLATFORM_HPP_

// Single source of truth for platform detection.
//
// Historically the code branched on PLATFORM_UNIX / PLATFORM_LINUX / WIN32,
// none of which were ever defined by the build system, so every non-Windows
// code path silently compiled out. Prefer the compiler-provided macros
// normalised here.
#if defined(_WIN32)
	#define LUGARU_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
	#define LUGARU_PLATFORM_MACOS 1
	#define LUGARU_PLATFORM_UNIX 1
#elif defined(__unix__)
	#define LUGARU_PLATFORM_UNIX 1
	#if defined(__linux__)
		#define LUGARU_PLATFORM_LINUX 1
	#endif
#else
	#error "Unsupported platform: could not identify the target OS."
#endif

#include <cfloat>
#include <cmath>
#include <cstdio>

#ifdef LUGARU_PLATFORM_WINDOWS
	#include <cstring>
	#ifndef strcasecmp
		#define strcasecmp(a, b) _stricmp(a, b)
	#endif
#endif

struct Point
{
    short v = 0;
    short h = 0;
};

typedef struct AbsoluteTime
{
    unsigned long hi;
    unsigned long lo;
} AbsoluteTime;

/* Returns time since the app started, not system start. */
AbsoluteTime UpTime();

typedef long Duration;

enum
{
    durationMicrosecond = -1,
    durationMillisecond = 1,
    durationSecond = 1000,
    durationMinute = 1000 * 60,
    durationHour = 1000 * 60 * 60,
    durationDay = 1000 * 60 * 60 * 24,
    durationForever = 0x7FFFFFFF,
    durationImmediate = 0,
};

Duration AbsoluteDeltaToDuration(AbsoluteTime& a, AbsoluteTime& b);

#endif // _PLATFORM_HPP_
