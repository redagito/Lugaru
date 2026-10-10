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

#ifndef _SKELETON_HPP_
#define _SKELETON_HPP_

#include "Animation/Joint.hpp"
#include "Animation/Muscle.hpp"

#include "Graphic/Models.hpp"

#include "Utils/Callbacks.h"
#include "Math/Vector3.hpp"

const int max_joints = 50;

class Terrain;

class Skeleton
{
public:
	std::vector<Joint> joints;
	std::vector<Muscle> muscles;

	int selected;

	int forwardjoints[3];
	Vector3 forward;

	int id;

	int lowforwardjoints[3];
	Vector3 lowforward;

	Vector3 specialforward[5];
	int jointlabels[max_joints];

	Model model[7];
	Model modellow;
	Model modelclothes;
	int num_models;

	Model drawmodel;
	Model drawmodellow;
	Model drawmodelclothes;

	bool clothes;
	bool spinny;

	// 768 KB of skin pixels, owned here and viewed through a raw pointer, so a
	// Skeleton no longer has to be the dominant cost of every Person that holds
	// one by value. The buffer is what the drawing code binds, and the size it
	// was last loaded at is written into skinsize.
	std::unique_ptr<GLubyte[]> skinText_storage;
	GLubyte* skinText = nullptr;
	int skinsize = 0;

	float checkdelay;

	float longdead;
	bool broken;

	int free;
	int oldfree;
	float freetime;
	bool freefall;

	void FindForwards();
	float DoConstraints(Vector3* coords, float* scale, bool tutorialActive, bool bloodtoggle, float timemultiplier, Terrain& terrain, int environment, float camerashake, bool freeze, int detail, int whichjointstartarray[26], int whichjointendarray[26]);
	void DoGravity(float* scale, float timemultiplier, float gravity);
	void FindRotationMuscle(int which, int animation);
	void Load(const std::string& fileName, const std::string& lowfileName, const std::string& clothesfileName, 
		const std::string& modelfileName, const std::string& model2fileName, const std::string& model3fileName, 
		const std::string& model4fileName, const std::string& model5fileNamee, const std::string& model6fileName, 
		const std::string& model7fileName, const std::string& modellowfileName, const std::string& modelclothesfileName, 
		bool aclothes, bool tutorialActive, ProgressCallback callback);

	Skeleton();

private:
	// convenience functions
	// only for Skeleton.cpp
	inline Joint& joint(int bodypart) { return joints[jointlabels[bodypart]]; }
	inline Vector3& jointPos(int bodypart) { return joint(bodypart).position; }
	inline Vector3& jointVel(int bodypart) { return joint(bodypart).velocity; }
};

#endif
