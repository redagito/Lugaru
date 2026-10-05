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
	gamestate.detail = 2;
	gamestate.ismotionblur = 1;
	gamestate.usermousesensitivity = 1;
	gamestate.newscreenwidth = kContextWidth = 1024;
	gamestate.newscreenheight = kContextHeight = 768;
	gamestate.fullscreen = 0;
	gamestate.floatjump = 0;
	gamestate.autoslomo = 1;
	gamestate.decalstoggle = true;
	gamestate.invertmouse = 0;
	gamestate.bloodtoggle = 0;
	gamestate.foliage = 1;
	gamestate.musictoggle = 1;
	gamestate.trilinear = 1;
	gamestate.gamespeed = 1;
	gamestate.damageeffects = 0;
	gamestate.texttoggle = 1;
	gamestate.alwaysblur = 0;
	gamestate.showpoints = 0;
	gamestate.showdamagebar = 0;
	gamestate.immediate = 0;
	gamestate.velocityblur = 0;
	gamestate.volume = 0.8f;
	gamestate.ambientsound = 1;
	gamestate.devtools = 0;

	gamestate.crouchkey = SDL_SCANCODE_LSHIFT;
	gamestate.jumpkey = SDL_SCANCODE_SPACE;
	gamestate.leftkey = SDL_SCANCODE_A;
	gamestate.forwardkey = SDL_SCANCODE_W;
	gamestate.backkey = SDL_SCANCODE_S;
	gamestate.rightkey = SDL_SCANCODE_D;
	gamestate.drawkey = SDL_SCANCODE_E;
	gamestate.throwkey = SDL_SCANCODE_Q;
	gamestate.attackkey = MOUSEBUTTON_LEFT;
	gamestate.consolekey = SDL_SCANCODE_GRAVE;

	gamestate.newdetail = gamestate.detail;
}

void SaveSettings(GameState& gamestate)
{
	if (gamestate.newdetail < 0) {
		gamestate.newdetail = 0;
	}
	if (gamestate.newdetail > 2) {
		gamestate.newdetail = 2;
	}
	if (gamestate.newscreenwidth < gamestate.minscreenwidth || gamestate.newscreenwidth > gamestate.maxscreenwidth) {
		gamestate.newscreenwidth = gamestate.screenwidth;
	}
	if (gamestate.newscreenheight < gamestate.minscreenheight || gamestate.newscreenheight > gamestate.maxscreenheight) {
		gamestate.newscreenheight = gamestate.screenheight;
	}
	errno = 0;
	std::ofstream opstream(Folders::getConfigFilePath());
	if (opstream.fail()) {
		perror(("Couldn't save config file " + Folders::getConfigFilePath()).c_str());
		return;
	}
	opstream << "Screenwidth:\n";
	opstream << gamestate.newscreenwidth;
	opstream << "\nScreenheight:\n";
	opstream << gamestate.newscreenheight;
	opstream << "\nFullscreen:\n";
	opstream << gamestate.fullscreen;
	opstream << "\nMouse sensitivity:\n";
	opstream << gamestate.usermousesensitivity;
	opstream << "\nBlur(0,1):\n";
	opstream << gamestate.ismotionblur;
	opstream << "\nOverall Detail(0,1,2) higher=better:\n";
	opstream << gamestate.newdetail;
	opstream << "\nFloating jump:\n";
	opstream << gamestate.floatjump;
	opstream << "\nMouse jump:\n";
	opstream << gamestate.mousejump;
	opstream << "\nAmbient sound:\n";
	opstream << gamestate.ambientsound;
	opstream << "\nBlood (0,1,2):\n";
	opstream << gamestate.bloodtoggle;
	opstream << "\nAuto slomo:\n";
	opstream << gamestate.autoslomo;
	opstream << "\nFoliage:\n";
	opstream << gamestate.foliage;
	opstream << "\nMusic:\n";
	opstream << gamestate.musictoggle;
	opstream << "\nTrilinear:\n";
	opstream << gamestate.trilinear;
	opstream << "\nDecals(shadows,blood puddles,etc):\n";
	opstream << gamestate.decalstoggle;
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
	opstream << gamestate.volume;
	opstream << "\nForward key:\n";
	opstream << gamestate.forwardkey;
	opstream << "\nBack key:\n";
	opstream << gamestate.backkey;
	opstream << "\nLeft key:\n";
	opstream << gamestate.leftkey;
	opstream << "\nRight key:\n";
	opstream << gamestate.rightkey;
	opstream << "\nJump key:\n";
	opstream << gamestate.jumpkey;
	opstream << "\nCrouch key:\n";
	opstream << gamestate.crouchkey;
	opstream << "\nDraw key:\n";
	opstream << gamestate.drawkey;
	opstream << "\nThrow key:\n";
	opstream << gamestate.throwkey;
	opstream << "\nAttack key:\n";
	opstream << gamestate.attackkey;
	opstream << "\nConsole key:\n";
	opstream << gamestate.consolekey;
	opstream << "\nDamage bar:\n";
	opstream << gamestate.showdamagebar;
	opstream << "\nStereoMode:\n";
	opstream << gamestate.stereomode;
	opstream << "\nStereoSeparation:\n";
	opstream << gamestate.stereoseparation;
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
			ipstream >> gamestate.fullscreen;
		}
		else if (!strncmp(setting, "Mouse sensitivity", 17)) {
			ipstream >> gamestate.usermousesensitivity;
		}
		else if (!strncmp(setting, "Blur", 4)) {
			ipstream >> gamestate.ismotionblur;
		}
		else if (!strncmp(setting, "Overall Detail", 14)) {
			ipstream >> gamestate.detail;
		}
		else if (!strncmp(setting, "Floating jump", 13)) {
			ipstream >> gamestate.floatjump;
		}
		else if (!strncmp(setting, "Mouse jump", 10)) {
			ipstream >> gamestate.mousejump;
		}
		else if (!strncmp(setting, "Ambient sound", 13)) {
			ipstream >> gamestate.ambientsound;
		}
		else if (!strncmp(setting, "Blood", 5)) {
			ipstream >> gamestate.bloodtoggle;
		}
		else if (!strncmp(setting, "Auto slomo", 10)) {
			ipstream >> gamestate.autoslomo;
		}
		else if (!strncmp(setting, "Foliage", 7)) {
			ipstream >> gamestate.foliage;
		}
		else if (!strncmp(setting, "Music", 5)) {
			ipstream >> gamestate.musictoggle;
		}
		else if (!strncmp(setting, "Trilinear", 9)) {
			ipstream >> gamestate.trilinear;
		}
		else if (!strncmp(setting, "Decals", 6)) {
			ipstream >> gamestate.decalstoggle;
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
			ipstream >> gamestate.devtools;
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
			ipstream >> gamestate.volume;
		}
		else if (!strncmp(setting, "Forward key", 11)) {
			ipstream >> gamestate.forwardkey;
		}
		else if (!strncmp(setting, "Back key", 8)) {
			ipstream >> gamestate.backkey;
		}
		else if (!strncmp(setting, "Left key", 8)) {
			ipstream >> gamestate.leftkey;
		}
		else if (!strncmp(setting, "Right key", 9)) {
			ipstream >> gamestate.rightkey;
		}
		else if (!strncmp(setting, "Jump key", 8)) {
			ipstream >> gamestate.jumpkey;
		}
		else if (!strncmp(setting, "Crouch key", 10)) {
			ipstream >> gamestate.crouchkey;
		}
		else if (!strncmp(setting, "Draw key", 8)) {
			ipstream >> gamestate.drawkey;
		}
		else if (!strncmp(setting, "Throw key", 9)) {
			ipstream >> gamestate.throwkey;
		}
		else if (!strncmp(setting, "Attack key", 10)) {
			ipstream >> gamestate.attackkey;
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
			gamestate.stereomode = (StereoMode)i;
		}
		else if (!strncmp(setting, "StereoSeparation", 16)) {
			ipstream >> gamestate.stereoseparation;
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

	if (gamestate.detail > 2) {
		gamestate.detail = 2;
	}
	if (gamestate.detail < 0) {
		gamestate.detail = 0;
	}
	if (gamestate.screenwidth < gamestate.minscreenwidth || gamestate.screenwidth > gamestate.maxscreenwidth) {
		gamestate.screenwidth = 1024;
	}
	if (gamestate.screenheight < gamestate.minscreenheight || gamestate.screenheight > gamestate.maxscreenheight) {
		gamestate.screenheight = 768;
	}

	gamestate.newdetail = gamestate.detail;
	return true;
}
