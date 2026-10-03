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

#ifndef _GAME_HPP_
#define _GAME_HPP_

#include "Animation/Skeleton.hpp"
#include "Audio/Sounds.hpp"
#include "Environment/Lights.hpp"
#include "Environment/Skybox.hpp"
#include "Environment/Terrain.hpp"
#include "Graphic/Models.hpp"
#include "Graphic/Sprite.hpp"
#include "Graphic/Stereo.hpp"
#include "Graphic/Text.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"
#include "Objects/Object.hpp"
#include "Objects/Person.hpp"
#include "Objects/Weapons.hpp"
#include "Thirdparty/optionparser.h"
#include "User/Account.hpp"
#include "Utils/ImageIO.hpp"
#include "Utils/binio.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <fstream>

#define NB_CAMPAIGN_MENU_ITEM 7

namespace Game 
{

	void newGame();
	void deleteGame();

	void InitGame();
	void LoadStuff();
	void LoadScreenTexture();
	void LoadingScreen();
	int DrawGLScene(StereoSide side);
	void playdialoguescenesound();
	int findClosestPlayer();
	void ResetBeforeLevelLoad(bool tutorial);
	bool LoadLevel(int which);
	bool LoadLevel(const std::string& name, bool tutorial = false);
	bool LoadJsonLevel(const std::string& name, bool tutorial = false);

	void cmd_dispatch(const std::string cmd);

	void ProcessInput();
	void ProcessDevInput();

	void Tick();
	void TickOnce();
	void TickOnceAfter();

	void SetUpLighting();
	GLvoid ReSizeGLScene(float fov, float near);

	void fireSound(int sound = fireendsound);

	void inputText(std::string& str, unsigned* charselected);
	void flash(float amount = 1, int delay = 1);
}

inline void swap_gl_buffers(void)
{
	extern SDL_Window* sdlwindow;
	SDL_GL_SwapWindow(sdlwindow);

	// try to limit this to 60fps, even if vsync fails.
	Uint32 now;
	static Uint32 frameticks = 0;
	const Uint32 endticks = (frameticks + 16);
	while ((now = SDL_GetTicks()) < endticks) { /* spin. */
	}
	frameticks = now;
}

enum maptypes
{
	mapkilleveryone,
	mapgosomewhere,
	mapkillsomeone,
	mapkillmost // These two are unused
};

enum pathtypes
{
	wpkeepwalking,
	wppause
};

extern const char* pathtypenames[2];

enum editortypes
{
	typeactive,
	typesitting,
	typesittingwall,
	typesleeping,
	typedead1,
	typedead2,
	typedead3,
	typedead4
};

extern const char* editortypenames[8];

#endif
