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

#ifndef _OBJECTS_HPP_
#define _OBJECTS_HPP_

#include "Environment/Lights.hpp"
#include "Environment/Terrain.hpp"

#include "Graphic/Models.hpp"
#include "Graphic/Sprite.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"

#include "Math/Frustum.hpp"
#include "Math/Vector3.hpp"
#include "Utils/ImageIO.hpp"
#include "Utils/Callbacks.h"

#include <memory>
#include <vector>
#include <json/value.h>
//
// Model Structures
//

#define max_objects 300

enum object_type
{
	boxtype = 0,
	weirdtype = 1,
	spiketype = 2,
	treetrunktype = 3,
	treeleavestype = 4,
	bushtype = 5,
	rocktype = 6,
	walltype = 7,
	chimneytype = 8,
	platformtype = 9,
	tunneltype = 11,
	cooltype = 12,
	firetype = 13
};

class Object
{
public:
	static std::vector<std::unique_ptr<Object>> objects;
	static Vector3 center;
	static float radius;
	static Texture boxtextureptr;
	static Texture treetextureptr;
	static Texture bushtextureptr;
	static Texture rocktextureptr;

	Vector3 position;
	object_type type = object_type::boxtype;
	float yaw = 0.f;
	float pitch = 0.f;
	float rotx = 0.f;
	float rotxvel = 0.f;
	float roty = 0.f;
	float rotyvel = 0.f;
	bool possible = false;
	Model model;
	Model displaymodel;
	float friction = 0.f;
	float scale = 0.f;
	float messedwith = 0.f;
	float checked = 0.f;
	float shadowed = 0.f;
	float occluded = 0.f;
	bool onfire = false;
	float flamedelay = 0.f;

	Object();
	Object(object_type _type, Vector3 _position, float _yaw, float _pitch, float _scale, const Terrain& terrain, ProgressCallback callback);
	Object(Json::Value, float, const Terrain& terrain, ProgressCallback callback);

	static void ComputeCenter();
	static void ComputeRadius();
	static void AddObjectsToTerrain(int environment, Terrain& terrain, int detail);
	static void LoadObjectsFromFile(FILE* tfile, bool skip, const Terrain& terrain, ProgressCallback callback);
	static void LoadObjectsFromJson(Json::Value, const Terrain& terrain, ProgressCallback callback);
	static void SphereCheckPossible(Vector3* p1, float checkradius, const Terrain& terrain);
	static void DeleteObject(int which, Terrain& terrain);
	static void MakeObject(int atype, Vector3 where, float ayaw, float apitch, float ascale, int environment, Terrain& terrain, bool foliage, int detail, ProgressCallback callback);
	static void Draw(bool decalstoggle, float multiplier, const Vector3& viewer, float viewdistance, float fadestart, int environment, 
		const Light& light, const Frustum& frustum, const Terrain& terrain, int detail, float blurness, float windvar, float playerdist);
	static void DoShadows(bool skyboxtexture, const Light& light, const Terrain& terrain);
	static void DoStuff(bool bloodtoggle, float multiplier);
	static int checkcollide(Vector3 startpoint, Vector3 endpoint, const Terrain& terrain);
	static int checkcollide(Vector3 startpoint, Vector3 endpoint, int what, const Terrain& terrain);

	operator Json::Value();

private:
	void handleFire(bool bloodtoggle, float multiplier);
	void handleRot(int divide, float multiplier);
	void doShadows(Vector3 lightloc, const Terrain& terrain);
	void draw(bool decalstoggle, float multiplier, const Vector3& viewer, float viewdistance, float fadestart, int environment, const Light& light, const Frustum& frustum, const Terrain& terrain, int detail, float blurness, float windvar, float playerdist);
	void drawSecondPass(const Vector3& viewer, int environment, float multiplier, const Frustum& frustum, const Terrain& terrain, float windvar, float playerdist);
	void addToTerrain(unsigned id, int environment, Terrain& terrain, int detail);
	static int checkcollide(Vector3 startpoint, Vector3 endpoint, int what, float minx, float miny, float minz, float maxx, float maxy, float maxz, const Terrain& terrain);
};

#endif
