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

#ifndef _DECAL_HPP_
#define _DECAL_HPP_

class Terrain;
class Model;

#include "Graphic/DecalType.h"
#include "Math/Vector3.hpp"

class Decal
{
public:
	Vector3 position;
	decal_type type = decal_type::shadowdecal;
	float opacity = 0.f;
	float rotation = 0.f;
	float alivetime = 0.f;
	float brightness = 0.f;

	// Fixed-size, so std::array rather than a C array: indexing a C array does not
	// check anything, and this is a quad whose vertices are computed from a terrain
	// tile index. A tile outside the range reaches these as an out-of-bounds write.
	// std::array plus .at() turns that into a throw instead of silent corruption.
	std::array<std::array<float, 2>, 3> texcoords;
	std::array<Vector3, 3> vertex;

	Decal();
	Decal(Vector3 position, decal_type type, float opacity, float rotation, float brightness, int whichx, int whichy, float size, const Terrain& terrain, bool first);
	Decal(Vector3 position, decal_type type, float opacity, float rotation, float size, const Model& model, int i, int which);
};

#endif
