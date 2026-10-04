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

#include "Globals.h"

// TODO GET RID OF ALL OF THESE!

float volume = 0;
bool ismotionblur = false;
float usermousesensitivity = 0;
bool decalstoggle = false;
float blurness = 0;
float windvar = 0;
int difficulty = 0;
float multiplier = 0;
float screenwidth = 0, screenheight = 0;
bool fullscreen = 0;
float viewdistance = 0;
Vector3 viewer;
Vector3 viewerfacing;
float fadestart = 0;
int environment = 0;
float texscale = 0;
float gravity = 0;
Light light;
Terrain terrain;

SDL_Window* sdlwindow;

int kTextureSize = 0;
int detail = 0;
Frustum frustum;
float texdetail = 0;
float realtexdetail = 0;
float playerdist = 0;
int slomo = 0;
int bloodtoggle = 0;
float camerashake = 0;
float woozy = 0;
float blackout = 0;
bool musictoggle = false;
bool trilinear;
Weapons weapons;
bool ambientsound = false;
float flashamount = 0;
Vector3 windvector;
int mainmenu = 0;
int whichjointstartarray[26] = { 0 };
int whichjointendarray[26] = { 0 };

float smoketex = 0;

int maptype = 0;

bool reversaltrain = false;
bool canattack = false;

bool skyboxtexture = false;
float skyboxr = 0;
float skyboxg = 0;
float skyboxb = 0;
float skyboxlightr = 0;
float skyboxlightg = 0;
float skyboxlightb = 0;

int hostile = 0;

bool devtools = false;

bool gamestarted = false;

StereoMode stereomode = stereoNone;
StereoMode newstereomode = stereoNone;
float stereoseparation = 0.05;
