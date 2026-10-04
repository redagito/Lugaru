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

#ifndef _OPENGL_INCLUDE_HPP_
#define _OPENGL_INCLUDE_HPP_

// Single source of truth for pulling in the platform OpenGL headers.
//
// This lives in Foundation because Foundation owns the platform layer and has to
// be self-contained: Foundation's include path is Foundation/include only, so
// anything it pulls in has to be reachable from there. Higher layers (Graphics
// via Graphic/gamegl.hpp, Game, App, Lugaru) include this header instead of
// keeping their own copy of the WIN32_LEAN_AND_MEAN / Polygon dance.
//
// Note that Foundation does not link OpenGL; on non-Windows this relies on the
// system GL development headers (GL/gl.h, GL/glext.h, GL/glu.h, or the
// OpenGL/ equivalents on macOS) being installed.

#include "Platform/Platform.hpp"

#ifdef LUGARU_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#define Polygon WinPolygon
	#include <windows.h>
	#undef Polygon
#endif

#define GL_GLEXT_PROTOTYPES 1
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
#endif

#endif