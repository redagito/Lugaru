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

#ifndef _SPRITE_HPP_
#define _SPRITE_HPP_

#include "Environment/Lights.hpp"
#include "Environment/Terrain.hpp"

#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"

#include "Math/Frustum.hpp"
#include "Math/Vector3.hpp"
#include "Utils/ImageIO.hpp"

#include <vector>

#define max_sprites 20000

enum
{
	cloudsprite = 0,
	bloodsprite,
	flamesprite,
	smoketype,
	weaponflamesprite,
	cloudimpactsprite,
	snowsprite,
	weaponshinesprite,
	bloodflamesprite,
	breathsprite,
	splintersprite,
	spritenumber
};

class Sprite
{
private:
	Vector3 oldposition;
	Vector3 position;
	Vector3 velocity;
	float size;
	float initialsize;
	int type;
	int special;
	float color[3];
	float opacity;
	float rotation;
	float alivetime;
	float speed;
	float rotatespeed;

	static float checkdelay;

	static std::vector<std::unique_ptr<Sprite> > sprites;

public:
	static void DeleteSprite(int which);
	static void MakeSprite(int atype, Vector3 where, Vector3 avelocity, float red, float green, float blue, float asize, float aopacity, bool bloodtoggle);

	// TODO Update and draw in one?
	static void Draw(const Vector3& viewer, float viewdistance, float fadestart, int environment, const Light& light, float multiplier,
		float gravity, Terrain& terrain, int detail, const Vector3& viewerfacing, bool bloodtoggle, const Vector3& windvector, bool tutorialActive);

	static void deleteSprites()
	{
		sprites.clear();
	}

	static void setLastSpriteSpecial(int s)
	{
		sprites.back()->special = s;
	}

	static void setLastSpriteSpeed(float s)
	{
		sprites.back()->speed = s;
	}

	static void setLastSpriteAlivetime(float al)
	{
		sprites.back()->alivetime = al;
	}

	static Texture cloudtexture;
	static Texture bloodtexture;
	static Texture flametexture;
	static Texture smoketexture;

	static Texture cloudimpacttexture;
	static Texture snowflaketexture;
	static Texture shinetexture;
	static Texture bloodflametexture;

	static Texture splintertexture;

	static Texture leaftexture;
	static Texture toothtexture;

	Sprite();
	~Sprite() = default;
};

#endif
