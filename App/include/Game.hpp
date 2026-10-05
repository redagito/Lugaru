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

struct GameAssets;
struct GameState;

namespace Game 
{

	void deleteGame(GameState& gamestate, GameAssets& assets);

	void InitGame(GameState& gamestate, GameAssets& assets);
	void LoadStuff(GameState& gamestate, GameAssets& assets);
	void LoadScreenTexture(GameState& gamestate, GameAssets& assets);
	void LoadingScreen(GameState& gamestate, GameAssets& assets);
	int DrawGLScene(StereoSide side, GameState& gamestate, GameAssets& assets);
	void playdialoguescenesound(GameState& gamestate);
	int findClosestPlayer();
	void ResetBeforeLevelLoad(bool tutorial, GameState& gamestate);
	bool LoadLevel(int which, GameState& gamestate, GameAssets& assets);
	bool LoadLevel(const std::string& name, bool tutorial, GameState& gamestate, GameAssets& assets);
	bool LoadJsonLevel(const std::string& name, bool tutorial, GameState& gamestate, GameAssets& assets);

	void cmd_dispatch(const std::string cmd, GameState& gamestate, GameAssets& assets);

	void ProcessInput(GameState& gamestate, GameAssets& assets);
	void ProcessDevInput(GameState& gamestate, GameAssets& assets);

	void Tick(GameState& gamestate, GameAssets& assets);
	void TickOnce(GameState& gamestate);
	void TickOnceAfter(GameState& gamestate, GameAssets& assets);

	void SetUpLighting(GameState& gamestate);
	GLvoid ReSizeGLScene(float fov, float near, GameState& gamestate);

	void fireSound(int sound = fireendsound);

	void inputText(std::string& str, unsigned* charselected, GameState& gamestate);
	void flash(GameState& gamestate, float amount = 1, int delay = 1);
}

// Presents the back buffer and waits out the rest of the frame. The window is
// passed in rather than reached for, so the caller names where it comes from -
// which is WindowContext, the only place the handle is declared.
inline void swap_gl_buffers(SDL_Window* window)
{
	SDL_GL_SwapWindow(window);

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
