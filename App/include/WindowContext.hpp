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

#ifndef _WINDOWCONTEXT_HPP_
#define _WINDOWCONTEXT_HPP_

#include <SDL.h>

#include <set>
#include <utility>

// Dimensions of the main window / GL context, in pixels. Owned by the
// application layer: the executable sets these up at startup and several
// subsystems (settings, menu, screenshots) read them.
extern int kContextWidth;
extern int kContextHeight;

struct GameState;

// Video modes reported by SDL, as (width, height) pairs. Populated during
// startup and consulted by the menu's resolution picker.
extern std::set<std::pair<int, int>> resolutions;

/** Toggles fullscreen mode on the main window. */
void toggleFullscreen(GameState& gamestate);

/**
 * Handles a single SDL event. Returns SDL_FALSE when the application should
 * quit, SDL_TRUE otherwise.
 */
SDL_bool sdlEventProc(const SDL_Event& e, GameState& gamestate);

#endif // _WINDOWCONTEXT_HPP_