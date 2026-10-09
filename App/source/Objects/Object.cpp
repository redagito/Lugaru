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

#include "GameAssets.hpp"
#include "Objects/Object.hpp"
#include <minwindef.h>
#include <gl/GL.h>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>
#include <GL/glext.h>
#include <Environment/Terrain.hpp>
#include <Graphic/Sprite.hpp>
#include <Math/Frustum.hpp>
#include <Math/Vector3.hpp>
#include <Utils/Callbacks.h>
#include <Environment/Lights.hpp>
#include <Graphic/Texture.hpp>
#include <cstdio>
#include <Utils/binio.h>
#include <json/value.h>
#include <Graphic/DecalType.h>

std::vector<std::unique_ptr<Object>> Object::objects;
Vector3 Object::center;
float Object::radius = 0;

//Functions

Object::Object()
	: position()
	, type(boxtype)
	, yaw(0)
	, pitch(0)
	, rotx(0)
	, rotxvel(0)
	, roty(0)
	, rotyvel(0)
	, possible(false)
	, model()
	, displaymodel()
	, friction(0)
	, scale(0)
	, messedwith(0)
	, checked(0)
	, shadowed(0)
	, occluded(0)
	, onfire(false)
	, flamedelay(0)
{
}

Object::Object(object_type _type, Vector3 _position, float _yaw, float _pitch, float _scale, const Terrain& terrain, ProgressCallback callback)
	: Object()
{
	scale = _scale;
	type = _type;
	position = _position;
	yaw = _yaw;
	pitch = _pitch;

	switch (type) {
	case boxtype:
		model.loaddecal("Models/Box.solid");
		friction = 1.5;
		break;
	case cooltype:
		model.loaddecal("Models/Cool.solid");
		friction = 1.5;
		break;
	case walltype:
		model.loaddecal("Models/Wall.solid");
		friction = 1.5;
		break;
	case tunneltype:
		model.loaddecal("Models/Tunnel.solid");
		friction = 1.5;
		break;
	case chimneytype:
		model.loaddecal("Models/Chimney.solid");
		friction = 1.5;
		break;
	case spiketype:
		model.load("Models/Spike.solid", callback);
		friction = .4;
		break;
	case weirdtype:
		model.loaddecal("Models/Weird.solid");
		friction = 1.5;
		break;
	case rocktype:
		model.loaddecal("Models/Rock.solid");
		if (scale > .5) {
			friction = 1.5;
		}
		else {
			friction = .5;
		}
		break;
	case treetrunktype:
		model.load("Models/TreeTrunk.solid", callback);
		friction = .4;
		break;
	case treeleavestype:
		scale += fabs((float)(rand() % 100) / 900) * scale;
		model.load("Models/Leaves.solid", callback);
		friction = 0;
		break;
	case bushtype:
		position.y = terrain.getHeight(position.x, position.z) - .3;
		model.load("Models/Bush.solid", callback);
		break;
	case platformtype:
		model.loaddecal("Models/Platform.solid");
		model.Rotate(90, 0, 0);
		friction = 1.5;
		break;
	case firetype:
		onfire = true;
		break;
	}

	if (friction == 1.5 && fabs(pitch) > 5) {
		friction = .5;
	}

	if (type == boxtype || type == cooltype || type == spiketype || type == weirdtype || type == walltype || type == chimneytype || type == tunneltype || type == platformtype) {
		model.ScaleTexCoords(scale * 1.5);
	}

	if (type == rocktype) {
		model.ScaleTexCoords(scale * 3);
	}

	model.flat = 1;
	if (type == treetrunktype || type == treeleavestype || type == rocktype) {
		model.flat = 0;
	}

	model.Scale(.3 * scale, .3 * scale, .3 * scale);
	model.Rotate(90, 1, 1);
	model.Rotate(pitch, 0, 0);

	if (type == rocktype) {
		model.Rotate(yaw * 5, 0, 0);
	}

	model.CalculateNormals(1, callback);
	model.ScaleNormals(-1, -1, -1);
}

void Object::handleFire(bool bloodtoggle, float multiplier)
{
	if (type == firetype) onfire = 1;
	if (!onfire) return;

	if ((type == bushtype) || (type == firetype)) {
		flamedelay -= multiplier * 3;
	}
	else if (type == treeleavestype) {
		flamedelay -= multiplier * 4;
	}

	Vector3 spawnpoint;
	while ((flamedelay < 0) && onfire) {
		flamedelay += .006;
		if ((type == bushtype) || (type == firetype)) {
			spawnpoint.x = ((float)(rand() % 100)) / 30 * scale;
			spawnpoint.y = ((float)(rand() % 100) + 60) / 30 * scale;
			spawnpoint.z = 0;
			spawnpoint = DoRotation(spawnpoint, 0, rand() % 360, 0);
			spawnpoint += position;
			Sprite::MakeSprite(flamesprite, spawnpoint, spawnpoint * 0, 1, 1, 1, (.6 + (float)abs(rand() % 100) / 200 - .25) * 5 * scale, 1, bloodtoggle);
		}
		else if (type == treeleavestype) {
			spawnpoint.x = ((float)(rand() % 100)) / 80 * scale;
			spawnpoint.y = ((float)(rand() % 100) + 80) / 12 * scale;
			spawnpoint.z = 0;
			spawnpoint = DoRotation(spawnpoint, 0, rand() % 360, 0);
			spawnpoint += position;
			Sprite::MakeSprite(flamesprite, spawnpoint, spawnpoint * 0, 1, 1, 1, (.6 + (float)abs(rand() % 100) / 200 - .25) * 6, 1, bloodtoggle);
		}
	}
}

void Object::doShadows(Vector3 lightloc, const Terrain& terrain)
{
	Vector3 testpoint, testpoint2, terrainpoint, col;
	int patchx, patchz;

	shadowed = 0;
	if (type == treeleavestype || type == treetrunktype || type == bushtype || type == firetype) return;

	for (int j = 0; j < model.vertexNum; j++) {
		terrainpoint = position + DoRotation(model.vertex[j] + model.normals[j] * .1, 0, yaw, 0);
		shadowed = 0;
		patchx = terrainpoint.x / (terrain.size / subdivision * terrain.scale);
		patchz = terrainpoint.z / (terrain.size / subdivision * terrain.scale);

		if (patchx >= 0 && patchz >= 0 && patchx < subdivision && patchz < subdivision) {
			for (unsigned int k = 0; k < terrain.patchobjects[patchx][patchz].size(); k++) {
				unsigned int l = terrain.patchobjects[patchx][patchz][k];
				if (objects[l]->type != treetrunktype) {
					testpoint = terrainpoint;
					testpoint2 = terrainpoint + lightloc * 50 * (1 - shadowed);
					if (objects[l]->model.LineCheck(&testpoint, &testpoint2, &col, &objects[l]->position, &objects[l]->yaw) != -1) {
						shadowed = 1 - (findDistance(&terrainpoint, &col) / 50);
					}
				}
			}
		}

		if (shadowed <= 0) continue;

		col = model.normals[j] - DoRotation(lightloc * shadowed, 0, -yaw, 0);
		Normalise(&col);
		for (unsigned int k = 0; k < model.Triangles.size(); k++) {
			if (model.Triangles[k].vertex[0] == j) {
				int l = k * 24;
				model.vArray[l + 2] = col.x;
				model.vArray[l + 3] = col.y;
				model.vArray[l + 4] = col.z;
			}

			if (model.Triangles[k].vertex[1] == j) {
				int l = k * 24;
				model.vArray[l + 10] = col.x;
				model.vArray[l + 11] = col.y;
				model.vArray[l + 12] = col.z;
			}

			if (model.Triangles[k].vertex[2] == j) {
				int l = k * 24;
				model.vArray[l + 18] = col.x;
				model.vArray[l + 19] = col.y;
				model.vArray[l + 20] = col.z;
			}
		}
	}

	shadowed = 0;
}

void Object::handleRot(int divide, float multiplier)
{
	messedwith -= multiplier;
	if (rotxvel || rotx) {
		if (rotx > 0) {
			rotxvel -= multiplier * 8 * fabs(rotx);
		}
		if (rotx < 0) {
			rotxvel += multiplier * 8 * fabs(rotx);
		}
		if (rotx > 0) {
			rotxvel -= multiplier * 4;
		}
		if (rotx < 0) {
			rotxvel += multiplier * 4;
		}
		if (rotxvel > 0) {
			rotxvel -= multiplier * 4;
		}
		if (rotxvel < 0) {
			rotxvel += multiplier * 4;
		}
		if (fabs(rotx) < multiplier * 4) {
			rotx = 0;
		}
		if (fabs(rotxvel) < multiplier * 4) {
			rotxvel = 0;
		}

		rotx += rotxvel * multiplier * 4;
	}

	if (rotyvel || roty) {
		if (roty > 0) {
			rotyvel -= multiplier * 8 * fabs(roty);
		}
		if (roty < 0) {
			rotyvel += multiplier * 8 * fabs(roty);
		}
		if (roty > 0) {
			rotyvel -= multiplier * 4;
		}
		if (roty < 0) {
			rotyvel += multiplier * 4;
		}
		if (rotyvel > 0) {
			rotyvel -= multiplier * 4;
		}
		if (rotyvel < 0) {
			rotyvel += multiplier * 4;
		}
		if (fabs(roty) < multiplier * 4) {
			roty = 0;
		}
		if (fabs(rotyvel) < multiplier * 4) {
			rotyvel = 0;
		}

		roty += rotyvel * multiplier * 4;
	}

	if (roty) {
		glRotatef(roty / divide, 1, 0, 0);
	}

	if (rotx) {
		glRotatef(-rotx / divide, 0, 0, 1);
	}

	// Clamp to -10, 10
	if (rotx > 10) {
		rotx = 10;
	}
	else if (rotx < -10) {
		rotx = -10;
	}

	if (roty > 10) {
		roty = 10;
	}
	else if (roty < -10) {
		roty = -10;
	}
}

void Object::draw(bool decalstoggle, float multiplier, const Vector3& viewer, float viewdistance, float fadestart, int environment, const Light& light, const Frustum& frustum, const Terrain& terrain, int detail, float blurness, float windvar, float playerdist, GameAssets& assets)
{
	float distance = 0.0;
	Vector3 moved, terrainlight;
	bool hidden = false;
	if (type == firetype) return;

	moved = DoRotation(model.boundingspherecenter, 0, yaw, 0);
	int sphereInFrustum = frustum.SphereInFrustum(position.x + moved.x, position.y + moved.y, position.z + moved.z, model.boundingsphereradius);
	if (type != tunneltype && !sphereInFrustum) return;

	distance = distsq(&viewer, &position);
	distance *= 1.2;
	hidden = !(distsqflat(&viewer, &position) > playerdist + 3 || (type != bushtype && type != treeleavestype));

	if (hidden) return;

	if (detail == 2 && distance > viewdistance * viewdistance / 4 && environment == desertenvironment) {
		glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, blurness);
	}
	else {
		glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
	}

	distance = (viewdistance * viewdistance - (distance - (viewdistance * viewdistance * fadestart)) * (1 / (1 - fadestart))) / viewdistance / viewdistance;
	if (distance > 1) distance = 1;

	if (distance <= 0) return;
	if (occluded >= 6) return;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	if (!model.color) {
		glEnable(GL_LIGHTING);
	}
	else {
		glDisable(GL_LIGHTING);
	}
	glDepthMask(1);
	glTranslatef(position.x, position.y, position.z);
	if (type == bushtype) {
		handleRot(1, multiplier);
	}

	if (type == treetrunktype || type == treeleavestype) {
		if (type == treetrunktype || environment == desertenvironment) {
			handleRot(6, multiplier);
		}
		else {
			handleRot(4, multiplier);
		}
	}

	if (environment == snowyenvironment) {
		if (type == treeleavestype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * 1.5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
		if (type == treetrunktype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
		if (type == bushtype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * 4 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
	}

	if (environment == grassyenvironment) {
		if (type == treeleavestype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * 1.5 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
		if (type == treetrunktype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * .5 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
		if (type == bushtype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * 4 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
	}

	if (environment == desertenvironment) {
		if (type == bushtype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * 4 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
	}
	glRotatef(yaw, 0, 1, 0);
	glColor4f((1 - shadowed) / 2 + .5, (1 - shadowed) / 2 + .5, (1 - shadowed) / 2 + .5, distance);

	if (distance >= 1) {
		glDisable(GL_BLEND);
		glAlphaFunc(GL_GREATER, 0.5);
	}
	else {
		glEnable(GL_BLEND);
		glAlphaFunc(GL_GREATER, 0.1);
	}

	if (type != treetrunktype && type != treeleavestype && type != bushtype && type != rocktype) {
		glEnable(GL_CULL_FACE);
		glAlphaFunc(GL_GREATER, 0.0001);
		model.drawdifftex(assets.boxtextureptr);
		model.drawdecals(terrain.shadowtexture, terrain.bloodtexture, terrain.bloodtexture2, terrain.breaktexture, decalstoggle, multiplier);
	}

	if (type == rocktype) {
		glEnable(GL_CULL_FACE);
		glAlphaFunc(GL_GREATER, 0.0001);
		glColor4f((1 - shadowed) / 2 + light.ambient[0], (1 - shadowed) / 2 + light.ambient[1], (1 - shadowed) / 2 + light.ambient[2], distance);
		model.drawdifftex(assets.rocktextureptr);
		model.drawdecals(terrain.shadowtexture, terrain.bloodtexture, terrain.bloodtexture2, terrain.breaktexture, decalstoggle, multiplier);
	}

	if (type == treeleavestype) {
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		terrainlight = terrain.getLighting(position.x, position.z);
		glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, distance);
		if (distance < 1) {
			glAlphaFunc(GL_GREATER, 0.2);
		}
		model.drawdifftex(assets.treetextureptr);
	}

	if (type == bushtype) {
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		terrainlight = terrain.getLighting(position.x, position.z);
		glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, distance);
		if (distance < 1) {
			glAlphaFunc(GL_GREATER, 0.2);
		}
		model.drawdifftex(assets.bushtextureptr);
	}

	if (type == treetrunktype) {
		glEnable(GL_CULL_FACE);
		terrainlight = terrain.getLighting(position.x, position.z);
		glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, distance);
		model.drawdifftex(assets.treetextureptr);
	}
	glPopMatrix();
}

void Object::drawSecondPass(const Vector3& viewer, int environment, float multiplier, const Frustum& frustum, const Terrain& terrain, float windvar, float playerdist, GameAssets& assets)
{
	static float distance;
	static Vector3 moved, terrainlight;
	bool hidden;
	if (type != treeleavestype && type != bushtype) return;

	moved = DoRotation(model.boundingspherecenter, 0, yaw, 0);

	if (!frustum.SphereInFrustum(position.x + moved.x, position.y + moved.y, position.z + moved.z, model.boundingsphereradius)) return;

	hidden = distsqflat(&viewer, &position) <= playerdist + 3;
	if (!hidden) return;

	distance = 1;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glEnable(GL_LIGHTING);
	glDepthMask(1);
	glTranslatef(position.x, position.y, position.z);

	if (type == bushtype) {
		handleRot(1, multiplier);
	}

	if (type == treetrunktype || type == treeleavestype) {
		handleRot(2, multiplier);
	}

	if (environment == snowyenvironment) {
		if (type == treeleavestype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * 1.5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
		if (type == treetrunktype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
		if (type == bushtype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * 4 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
	}

	if (environment == grassyenvironment) {
		if (type == treeleavestype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * 1.5 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
		if (type == treetrunktype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * .5 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
		if (type == bushtype) {
			glRotatef((sin(windvar + position.x * .3) + .5) * 4 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
		}
	}

	glRotatef(yaw, 0, 1, 0);
	glColor4f(1, 1, 1, distance);

	if (type == treeleavestype) {
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		terrainlight = terrain.getLighting(position.x, position.z);
		glDepthMask(0);
		glEnable(GL_BLEND);
		glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, .3);
		glAlphaFunc(GL_GREATER, 0);
		glDisable(GL_ALPHA_TEST);
		model.drawdifftex(assets.treetextureptr);
	}

	if (type == bushtype) {
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		terrainlight = terrain.getLighting(position.x, position.z);
		glDepthMask(0);
		glEnable(GL_BLEND);
		glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, .3);
		glAlphaFunc(GL_GREATER, 0);
		glDisable(GL_ALPHA_TEST);
		model.drawdifftex(assets.bushtextureptr);
	}

	glPopMatrix();
}

void Object::ComputeCenter()
{
	center = 0;
	for (unsigned i = 0; i < objects.size(); i++) {
		center += objects[i]->position;
	}
	center /= objects.size();
}

void Object::ComputeRadius()
{
	float maxdistance = 0;
	float tempdist;
	for (unsigned int i = 0; i < objects.size(); i++) {
		tempdist = distsq(&center, &objects[i]->position);
		if (tempdist > maxdistance) {
			maxdistance = tempdist;
		}
	}
	radius = sqrt(maxdistance);
}

void Object::LoadObjectsFromFile(FILE* tfile, bool skip, const Terrain& terrain, ProgressCallback callback)
{
	int numobjects;
	int type;
	Vector3 position;
	float yaw, pitch, scale;
	float lastscale = 1.0f;
	funpackf(tfile, "Bi", &numobjects);
	if (!skip) {
		objects.clear();
	}

	for (int i = 0; i < numobjects; i++) {
		funpackf(tfile, "Bi Bf Bf Bf Bf Bf Bf", &type, &yaw, &pitch, &position.x, &position.y, &position.z, &scale);

		if (skip) continue;

		if (type == treeleavestype) {
			scale = lastscale;
		}
		objects.emplace_back(new Object(object_type(type), position, yaw, pitch, scale, terrain, callback));
		lastscale = scale;
	}
}

void Object::LoadObjectsFromJson(Json::Value values, const Terrain& terrain, ProgressCallback callback)
{
	objects.clear();
	float lastscale = 1.0f;
	for (unsigned i = 0; i < values.size(); i++) {
		objects.emplace_back(new Object(values[i], lastscale, terrain, callback));
		lastscale = objects.back()->scale;
	}
}

void Object::addToTerrain(unsigned id, int environment, Terrain& terrain, int detail)
{
	if ((type != treeleavestype) && (type != bushtype) && (type != firetype)) {
		terrain.AddObject(position + DoRotation(model.boundingspherecenter, 0, yaw, 0), model.boundingsphereradius, id);
	}

	if (detail != 2) return;

	if ((type == treetrunktype) && (position.y < (terrain.getHeight(position.x, position.z) + 1))) {
		terrain.MakeDecal(shadowdecalpermanent, position, 2, .4, 0, environment);
	}

	if ((type == bushtype) && (position.y < (terrain.getHeight(position.x, position.z) + 1))) {
		terrain.MakeDecal(shadowdecalpermanent, position, 1, .4, 0, environment);
	}
}

void Object::AddObjectsToTerrain(int environment, Terrain& terrain, int detail)
{
	for (unsigned i = 0; i < objects.size(); i++) {
		objects[i]->addToTerrain(i, environment, terrain, detail);
	}
}

void Object::SphereCheckPossible(Vector3* p1, float checkradius, const Terrain& terrain)
{
	int whichpatchx = p1->x / (terrain.size / subdivision * terrain.scale);
	int whichpatchz = p1->z / (terrain.size / subdivision * terrain.scale);

	if (whichpatchx < 0) return;
	if (whichpatchx >= subdivision) return;
	if (whichpatchz < 0) return;
	if (whichpatchz >= subdivision) return;
	if (terrain.patchobjects[whichpatchx][whichpatchz].size() >= 500) return;

	for (unsigned int j = 0; j < terrain.patchobjects[whichpatchx][whichpatchz].size(); j++) {
		unsigned int i = terrain.patchobjects[whichpatchx][whichpatchz][j];
		objects[i]->possible = false;

		if (objects[i]->model.SphereCheckPossible(p1, checkradius, &objects[i]->position, &objects[i]->yaw) != -1) {
			objects[i]->possible = true;
		}
	}
}

void Object::Draw(bool decalstoggle, float multiplier, const Vector3& viewer, float viewdistance, float fadestart, int environment,
	const Light& light, const Frustum& frustum, const Terrain& terrain, int detail, float blurness, float windvar, float playerdist, GameAssets& assets)
{
	for (unsigned i = 0; i < objects.size(); i++) {
		objects[i]->draw(decalstoggle, multiplier, viewer, viewdistance, fadestart, environment, light, frustum, terrain, detail, blurness, windvar, playerdist, assets);
	}

	glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
	for (unsigned i = 0; i < objects.size(); i++) {
		objects[i]->drawSecondPass(viewer, environment, multiplier, frustum, terrain, windvar, playerdist, assets);
	}
	if (environment == desertenvironment) {
		glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
	}
	glEnable(GL_ALPHA_TEST);
	SetUpLight(&light, 0);
}

void Object::DeleteObject(int which, Terrain& terrain)
{
	objects.erase(objects.begin() + which);
	terrain.DeleteObject(which);
}

void Object::MakeObject(int atype, Vector3 where, float ayaw, float apitch, float ascale, int environment, Terrain& terrain, bool foliage, int detail, ProgressCallback callback)
{
	if ((atype != treeleavestype && atype != bushtype) || foliage == 1) {
		unsigned nextid = objects.size();
		objects.emplace_back(new Object(object_type(atype), where, ayaw, apitch, ascale, terrain, callback));
		objects.back()->addToTerrain(nextid, environment, terrain, detail);
	}
}

void Object::DoStuff(bool bloodtoggle, float multiplier)
{
	for (unsigned i = 0; i < objects.size(); i++) {
		objects[i]->handleFire(bloodtoggle, multiplier);
	}
}

void Object::DoShadows(bool skyboxtexture, const Light& light, const Terrain& terrain)
{
	Vector3 lightloc;
	lightloc = light.location;
	if (!skyboxtexture) {
		lightloc = 0;
	}
	lightloc.y += 10;
	Normalise(&lightloc);

	for (unsigned i = 0; i < objects.size(); i++) {
		objects[i]->doShadows(lightloc, terrain);
	}
}

int Object::checkcollide(Vector3 startpoint, Vector3 endpoint, const Terrain& terrain)
{
	float minx, minz, maxx, maxz, miny, maxy;

	minx = min(startpoint.x, endpoint.x) - 1;
	miny = min(startpoint.y, endpoint.y) - 1;
	minz = min(startpoint.z, endpoint.z) - 1;
	maxx = max(startpoint.x, endpoint.x) + 1;
	maxy = max(startpoint.y, endpoint.y) + 1;
	maxz = max(startpoint.z, endpoint.z) + 1;

	for (unsigned int i = 0; i < objects.size(); i++) {
		if (checkcollide(startpoint, endpoint, i, minx, miny, minz, maxx, maxy, maxz, terrain) != -1) {
			return (int)i;
		}
	}

	return -1;
}

int Object::checkcollide(Vector3 startpoint, Vector3 endpoint, int what, const Terrain& terrain)
{
	float minx, minz, maxx, maxz, miny, maxy;

	minx = min(startpoint.x, endpoint.x) - 1;
	miny = min(startpoint.y, endpoint.y) - 1;
	minz = min(startpoint.z, endpoint.z) - 1;
	maxx = max(startpoint.x, endpoint.x) + 1;
	maxy = max(startpoint.y, endpoint.y) + 1;
	maxz = max(startpoint.z, endpoint.z) + 1;

	return checkcollide(startpoint, endpoint, what, minx, miny, minz, maxx, maxy, maxz, terrain);
}

int Object::checkcollide(Vector3 startpoint, Vector3 endpoint, int what, float minx, float miny, float minz, float maxx, float maxy, float maxz, const Terrain& terrain)
{
	Vector3 colpoint, colviewer, coltarget;

	if (what == 1000) {
		if (terrain.lineTerrain(startpoint, endpoint, &colpoint) != -1) {
			return what;
		}
	}
	else {
		if (objects[what]->position.x > minx - objects[what]->model.boundingsphereradius &&
			objects[what]->position.x < maxx + objects[what]->model.boundingsphereradius &&
			objects[what]->position.y > miny - objects[what]->model.boundingsphereradius &&
			objects[what]->position.y < maxy + objects[what]->model.boundingsphereradius &&
			objects[what]->position.z > minz - objects[what]->model.boundingsphereradius &&
			objects[what]->position.z < maxz + objects[what]->model.boundingsphereradius) {
			
			if (objects[what]->type != treeleavestype &&
				objects[what]->type != bushtype &&
				objects[what]->type != firetype) {
				colviewer = startpoint;
				coltarget = endpoint;

				if (objects[what]->model.LineCheck(&colviewer, &coltarget, &colpoint, &objects[what]->position, &objects[what]->yaw) != -1) {
					return what;
				}
			}
		}
	}

	return -1;
}

Object::Object(Json::Value value, float lastscale, const Terrain& terrain, ProgressCallback callback)
	: Object(object_type(value[0].asInt()), value[4], value[1].asFloat(), value[2].asFloat(), ((value[0].asInt() == treeleavestype) ? lastscale : value[3].asFloat()), terrain, callback)
{
}

Object::operator Json::Value() {
	Json::Value object;

	object[0] = type;
	object[1] = yaw;
	object[2] = pitch;
	if (type == treeleavestype) {
		/* Store 1.0 as scale is ignored anyway for tree leaves.
		 * This avoids having random floats in the output to make sure the same map gives the same json each time. */
		object[3] = 1.0f;
	}
	else {
		object[3] = scale;
	}
	object[4] = position;

	return object;
}

