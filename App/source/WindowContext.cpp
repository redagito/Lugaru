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
#include "GameState.hpp"
#include "Globals.h"

using namespace Game;

namespace WindowContext
{

int kContextWidth = 0;
int kContextHeight = 0;

std::set<std::pair<int, int>> resolutions;

// The one and only window. File-scope static, so it has internal linkage:
// nothing outside this translation unit can declare it, which is what stops the
// extern in a header and the global this replaced from coming back.
static SDL_Window* sdlwindow = nullptr;

SDL_Window* createWindow(int width, int height, Uint32 flags)
{
	sdlwindow = SDL_CreateWindow("Lugaru", SDL_WINDOWPOS_CENTERED_DISPLAY(0), SDL_WINDOWPOS_CENTERED_DISPLAY(0), width, height, flags);
	return sdlwindow;
}

SDL_Window* mainWindow()
{
	return sdlwindow;
}

bool createGLContext()
{
	SDL_GLContext glctx = SDL_GL_CreateContext(sdlwindow);
	if (!glctx) {
		return false;
	}

	SDL_GL_MakeCurrent(sdlwindow, glctx);
	return true;
}

bool isFocused()
{
	return ((SDL_GetWindowFlags(sdlwindow) & SDL_WINDOW_INPUT_FOCUS) != 0);
}

void toggleFullscreen(GameState& gamestate)
{
	gamestate.fullscreen = !gamestate.fullscreen;
	Uint32 flags = SDL_GetWindowFlags(sdlwindow);
	if (flags & SDL_WINDOW_FULLSCREEN) {
		flags &= ~SDL_WINDOW_FULLSCREEN;
	}
	else {
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	SDL_SetWindowFullscreen(sdlwindow, flags);
}

SDL_bool sdlEventProc(const SDL_Event& e, GameState& gamestate)
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
		gamestate.deltah += e.motion.xrel;
		gamestate.deltav += e.motion.yrel;
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
			toggleFullscreen(gamestate);
		}
		break;
	}
	return SDL_TRUE;
}

}