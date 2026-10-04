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

float multiplier = 0;
Vector3 viewer;
Vector3 viewerfacing;
Light light;
Terrain terrain;

SDL_Window* sdlwindow;

Frustum frustum;
Weapons weapons;
Vector3 windvector;
int whichjointstartarray[26] = { 0 };
int whichjointendarray[26] = { 0 };

StereoMode stereomode = stereoNone;
StereoMode newstereomode = stereoNone;
