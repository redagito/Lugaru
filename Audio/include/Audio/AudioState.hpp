/*
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

#ifndef _AUDIOSTATE_HPP_
#define _AUDIOSTATE_HPP_

#include "Math/Vector3.hpp"

/** Capacity of the pool of positional ambient sound sources. */
constexpr int max_env_sounds = 30;

/**
 * Active ambient sound sources, stored as parallel arrays indexed by
 * [0, numenvsounds). Populated via addEnvSound(); entries are aged and
 * retired by the game tick.
 */
extern Vector3 envsound[max_env_sounds];
extern float envsoundvol[max_env_sounds];
extern float envsoundlife[max_env_sounds];
extern int numenvsounds;

/**
 * Playback pitch divisor used while the game is in slow motion. Applied as
 * AL_PITCH = slomofreq / 44100.
 */
extern float slomofreq;

#endif // _AUDIOSTATE_HPP_