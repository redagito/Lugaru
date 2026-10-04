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

#include "User/Settings.hpp"

#include "GameGlobals.h"
#include "GameState.hpp"
#include "Globals.h"
#include "Graphic/Stereo.hpp"
#include "Utils/Folders.hpp"
#include "Utils/Input.hpp"
#include "WindowContext.hpp"

void DefaultSettings(GameState& gamestate)
{
	detail = 2;
	ismotionblur = 1;
	usermousesensitivity = 1;
	Game::newscreenwidth = kContextWidth = 1024;
	Game::newscreenheight = kContextHeight = 768;
	fullscreen = 0;
	gamestate.floatjump = 0;
	gamestate.autoslomo = 1;
	decalstoggle = true;
	gamestate.invertmouse = 0;
	bloodtoggle = 0;
	gamestate.foliage = 1;
	musictoggle = 1;
	trilinear = 1;
	gamestate.gamespeed = 1;
	gamestate.damageeffects = 0;
	gamestate.texttoggle = 1;
	gamestate.alwaysblur = 0;
	gamestate.showpoints = 0;
	gamestate.showdamagebar = 0;
	gamestate.immediate = 0;
	gamestate.velocityblur = 0;
	volume = 0.8f;
	ambientsound = 1;
	devtools = 0;

	Game::crouchkey = SDL_SCANCODE_LSHIFT;
	Game::jumpkey = SDL_SCANCODE_SPACE;
	Game::leftkey = SDL_SCANCODE_A;
	Game::forwardkey = SDL_SCANCODE_W;
	Game::backkey = SDL_SCANCODE_S;
	Game::rightkey = SDL_SCANCODE_D;
	Game::drawkey = SDL_SCANCODE_E;
	Game::throwkey = SDL_SCANCODE_Q;
	Game::attackkey = MOUSEBUTTON_LEFT;
	gamestate.consolekey = SDL_SCANCODE_GRAVE;

	Game::newdetail = detail;
}

void SaveSettings(GameState& gamestate)
{
	if (Game::newdetail < 0) {
		Game::newdetail = 0;
	}
	if (Game::newdetail > 2) {
		Game::newdetail = 2;
	}
	if (Game::newscreenwidth < gamestate.minscreenwidth || Game::newscreenwidth > gamestate.maxscreenwidth) {
		Game::newscreenwidth = screenwidth;
	}
	if (Game::newscreenheight < gamestate.minscreenheight || Game::newscreenheight > gamestate.maxscreenheight) {
		Game::newscreenheight = screenheight;
	}
	errno = 0;
	std::ofstream opstream(Folders::getConfigFilePath());
	if (opstream.fail()) {
		perror(("Couldn't save config file " + Folders::getConfigFilePath()).c_str());
		return;
	}
	opstream << "Screenwidth:\n";
	opstream << Game::newscreenwidth;
	opstream << "\nScreenheight:\n";
	opstream << Game::newscreenheight;
	opstream << "\nFullscreen:\n";
	opstream << fullscreen;
	opstream << "\nMouse sensitivity:\n";
	opstream << usermousesensitivity;
	opstream << "\nBlur(0,1):\n";
	opstream << ismotionblur;
	opstream << "\nOverall Detail(0,1,2) higher=better:\n";
	opstream << Game::newdetail;
	opstream << "\nFloating jump:\n";
	opstream << gamestate.floatjump;
	opstream << "\nMouse jump:\n";
	opstream << gamestate.mousejump;
	opstream << "\nAmbient sound:\n";
	opstream << ambientsound;
	opstream << "\nBlood (0,1,2):\n";
	opstream << bloodtoggle;
	opstream << "\nAuto slomo:\n";
	opstream << gamestate.autoslomo;
	opstream << "\nFoliage:\n";
	opstream << gamestate.foliage;
	opstream << "\nMusic:\n";
	opstream << musictoggle;
	opstream << "\nTrilinear:\n";
	opstream << trilinear;
	opstream << "\nDecals(shadows,blood puddles,etc):\n";
	opstream << decalstoggle;
	opstream << "\nInvert mouse:\n";
	opstream << gamestate.invertmouse;
	opstream << "\nGamespeed:\n";
	if (gamestate.oldgamespeed == 0) {
		gamestate.oldgamespeed = 1;
	}
	opstream << gamestate.oldgamespeed;
	opstream << "\nDamage effects(blackout, doublevision):\n";
	opstream << gamestate.damageeffects;
	opstream << "\nText:\n";
	opstream << gamestate.texttoggle;
	opstream << "\nShow Points:\n";
	opstream << gamestate.showpoints;
	opstream << "\nAlways Blur:\n";
	opstream << gamestate.alwaysblur;
	opstream << "\nImmediate mode (turn on on G5):\n";
	opstream << gamestate.immediate;
	opstream << "\nVelocity blur:\n";
	opstream << gamestate.velocityblur;
	opstream << "\nVolume:\n";
	opstream << volume;
	opstream << "\nForward key:\n";
	opstream << Game::forwardkey;
	opstream << "\nBack key:\n";
	opstream << Game::backkey;
	opstream << "\nLeft key:\n";
	opstream << Game::leftkey;
	opstream << "\nRight key:\n";
	opstream << Game::rightkey;
	opstream << "\nJump key:\n";
	opstream << Game::jumpkey;
	opstream << "\nCrouch key:\n";
	opstream << Game::crouchkey;
	opstream << "\nDraw key:\n";
	opstream << Game::drawkey;
	opstream << "\nThrow key:\n";
	opstream << Game::throwkey;
	opstream << "\nAttack key:\n";
	opstream << Game::attackkey;
	opstream << "\nConsole key:\n";
	opstream << gamestate.consolekey;
	opstream << "\nDamage bar:\n";
	opstream << gamestate.showdamagebar;
	opstream << "\nStereoMode:\n";
	opstream << stereomode;
	opstream << "\nStereoSeparation:\n";
	opstream << stereoseparation;
	opstream << "\nStereoReverse:\n";
	opstream << gamestate.stereoreverse;
	opstream << "\n";
	opstream.close();
}

bool LoadSettings(GameState& gamestate)
{
	errno = 0;
	std::ifstream ipstream(Folders::getConfigFilePath(), std::ios::in);
	if (ipstream.fail()) {
		perror(("Couldn't read config file " + Folders::getConfigFilePath()).c_str());
		return false;
	}
	char setting[256];
	char string[256];

	printf("Loading config\n");
	while (!ipstream.eof()) {
		ipstream.getline(setting, sizeof(setting));

		// skip blank lines
		// assume lines starting with spaces are all blank
		if (strlen(setting) == 0 || setting[0] == ' ' || setting[0] == '\t') {
			continue;
		}
		//~ printf("setting : %s\n",setting);

		if (ipstream.eof() || ipstream.fail()) {
			fprintf(stderr, "Error reading config file: Got setting name '%s', but value can't be read\n", setting);
			ipstream.close();
			return false;
		}

		if (!strncmp(setting, "Screenwidth", 11)) {
			ipstream >> kContextWidth;
			if (kContextWidth < (int)gamestate.minscreenwidth || kContextWidth >(int)gamestate.maxscreenwidth) {
				kContextWidth = (int)gamestate.minscreenwidth;
			}
		}
		else if (!strncmp(setting, "Screenheight", 12)) {
			ipstream >> kContextHeight;
			if (kContextHeight < (int)gamestate.minscreenheight || kContextHeight >(int)gamestate.maxscreenheight) {
				kContextHeight = (int)gamestate.minscreenheight;
			}
		}
		else if (!strncmp(setting, "Fullscreen", 10)) {
			ipstream >> fullscreen;
		}
		else if (!strncmp(setting, "Mouse sensitivity", 17)) {
			ipstream >> usermousesensitivity;
		}
		else if (!strncmp(setting, "Blur", 4)) {
			ipstream >> ismotionblur;
		}
		else if (!strncmp(setting, "Overall Detail", 14)) {
			ipstream >> detail;
		}
		else if (!strncmp(setting, "Floating jump", 13)) {
			ipstream >> gamestate.floatjump;
		}
		else if (!strncmp(setting, "Mouse jump", 10)) {
			ipstream >> gamestate.mousejump;
		}
		else if (!strncmp(setting, "Ambient sound", 13)) {
			ipstream >> ambientsound;
		}
		else if (!strncmp(setting, "Blood", 5)) {
			ipstream >> bloodtoggle;
		}
		else if (!strncmp(setting, "Auto slomo", 10)) {
			ipstream >> gamestate.autoslomo;
		}
		else if (!strncmp(setting, "Foliage", 7)) {
			ipstream >> gamestate.foliage;
		}
		else if (!strncmp(setting, "Music", 5)) {
			ipstream >> musictoggle;
		}
		else if (!strncmp(setting, "Trilinear", 9)) {
			ipstream >> trilinear;
		}
		else if (!strncmp(setting, "Decals", 6)) {
			ipstream >> decalstoggle;
		}
		else if (!strncmp(setting, "Invert mouse", 12)) {
			ipstream >> gamestate.invertmouse;
		}
		else if (!strncmp(setting, "Gamespeed", 9)) {
			ipstream >> gamestate.gamespeed;
			gamestate.oldgamespeed = gamestate.gamespeed;
			if (gamestate.oldgamespeed == 0) {
				gamestate.gamespeed = 1;
				gamestate.oldgamespeed = 1;
			}
		}
		else if (!strncmp(setting, "Damage effects", 14)) {
			ipstream >> gamestate.damageeffects;
		}
		else if (!strncmp(setting, "Text", 4)) {
			ipstream >> gamestate.texttoggle;
		}
		else if (!strncmp(setting, "Devtools", 8)) {
			ipstream >> devtools;
		}
		else if (!strncmp(setting, "Show Points", 11)) {
			ipstream >> gamestate.showpoints;
		}
		else if (!strncmp(setting, "Always Blur", 11)) {
			ipstream >> gamestate.alwaysblur;
		}
		else if (!strncmp(setting, "Immediate mode ", 15)) {
			ipstream >> gamestate.immediate;
		}
		else if (!strncmp(setting, "Velocity blur", 13)) {
			ipstream >> gamestate.velocityblur;
		}
		else if (!strncmp(setting, "Volume", 6)) {
			ipstream >> volume;
		}
		else if (!strncmp(setting, "Forward key", 11)) {
			ipstream >> Game::forwardkey;
		}
		else if (!strncmp(setting, "Back key", 8)) {
			ipstream >> Game::backkey;
		}
		else if (!strncmp(setting, "Left key", 8)) {
			ipstream >> Game::leftkey;
		}
		else if (!strncmp(setting, "Right key", 9)) {
			ipstream >> Game::rightkey;
		}
		else if (!strncmp(setting, "Jump key", 8)) {
			ipstream >> Game::jumpkey;
		}
		else if (!strncmp(setting, "Crouch key", 10)) {
			ipstream >> Game::crouchkey;
		}
		else if (!strncmp(setting, "Draw key", 8)) {
			ipstream >> Game::drawkey;
		}
		else if (!strncmp(setting, "Throw key", 9)) {
			ipstream >> Game::throwkey;
		}
		else if (!strncmp(setting, "Attack key", 10)) {
			ipstream >> Game::attackkey;
		}
		else if (!strncmp(setting, "Console key", 11)) {
			ipstream >> gamestate.consolekey;
		}
		else if (!strncmp(setting, "Damage bar", 10)) {
			ipstream >> gamestate.showdamagebar;
		}
		else if (!strncmp(setting, "StereoMode", 10)) {
			int i;
			ipstream >> i;
			stereomode = (StereoMode)i;
		}
		else if (!strncmp(setting, "StereoSeparation", 16)) {
			ipstream >> stereoseparation;
		}
		else if (!strncmp(setting, "StereoReverse", 13)) {
			ipstream >> gamestate.stereoreverse;
		}
		else {
			ipstream >> string;
			fprintf(stderr, "Unknown config option '%s' with value '%s'. Ignoring.\n", setting, string);
		}

		if (ipstream.fail()) {
			fprintf(stderr, "Error reading config file: EOF reached when trying to read value for setting '%s'.\n", setting);
			ipstream.close();
			return false;
		}

		if (ipstream.bad()) {
			fprintf(stderr, "Error reading config file: Failed to read value for setting '%s'.\n", setting);
			ipstream.close();
			return false;
		}
	}

	ipstream.close();

	if (detail > 2) {
		detail = 2;
	}
	if (detail < 0) {
		detail = 0;
	}
	if (screenwidth < gamestate.minscreenwidth || screenwidth > gamestate.maxscreenwidth) {
		screenwidth = 1024;
	}
	if (screenheight < gamestate.minscreenheight || screenheight > gamestate.maxscreenheight) {
		screenheight = 768;
	}

	Game::newdetail = detail;
	return true;
}
