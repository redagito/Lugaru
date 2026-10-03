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

#include "WindowContext.hpp"

#include "GameGlobals.h"
#include "Globals.h"

using namespace Game;

int kContextWidth = 0;
int kContextHeight = 0;

std::set<std::pair<int, int>> resolutions;

void toggleFullscreen()
{
	fullscreen = !fullscreen;
	Uint32 flags = SDL_GetWindowFlags(sdlwindow);
	if (flags & SDL_WINDOW_FULLSCREEN) {
		flags &= ~SDL_WINDOW_FULLSCREEN;
	}
	else {
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	SDL_SetWindowFullscreen(sdlwindow, flags);
}

SDL_bool sdlEventProc(const SDL_Event& e)
{
	switch (e.type) {
	case SDL_QUIT:
		return SDL_FALSE;

	case SDL_WINDOWEVENT:
		if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
			return SDL_FALSE;
		}
		break;

	case SDL_MOUSEMOTION:
		deltah += e.motion.xrel;
		deltav += e.motion.yrel;
		break;

	case SDL_KEYDOWN:
		if ((e.key.keysym.scancode == SDL_SCANCODE_G) &&
			(e.key.keysym.mod & KMOD_CTRL)) {
			SDL_bool mode = SDL_TRUE;
			if ((SDL_GetWindowFlags(sdlwindow) & SDL_WINDOW_FULLSCREEN) == 0) {
				mode = (SDL_GetWindowGrab(sdlwindow) ? SDL_FALSE : SDL_TRUE);
			}
			SDL_SetWindowGrab(sdlwindow, mode);
			SDL_SetRelativeMouseMode(mode);
		}
		else if ((e.key.keysym.scancode == SDL_SCANCODE_RETURN) && (e.key.keysym.mod & KMOD_ALT)) {
			toggleFullscreen();
		}
		break;
	}
	return SDL_TRUE;
}