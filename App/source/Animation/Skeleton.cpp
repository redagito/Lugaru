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

#include <cmath>

#include "Animation/Skeleton.hpp"
#include "Environment/Terrain.hpp"

#include "Objects/Object.hpp"
#include "Graphic/Sprite.hpp"

// Graphics
#include "Animation/Animation.hpp"
// Audio
#include "Audio/Sounds.hpp"
// Foundation
#include "Utils/Folders.hpp"

	Skeleton::Skeleton()
	: selected(0)
	, id(0)
	, num_models(0)
	, clothes(false)
	, spinny(false)
	, skinsize(0)
	, checkdelay(0)
	, longdead(0)
	, broken(false)
	, free(0)
	, oldfree(0)
	, freetime(0)
	, freefall(false)
{
	memset(forwardjoints, 0, sizeof(forwardjoints));
	memset(lowforwardjoints, 0, sizeof(lowforwardjoints));
	memset(jointlabels, 0, sizeof(jointlabels));

	skinText_storage.reset(new GLubyte[512 * 512 * 3]());
	skinText = skinText_storage.get();
}

Joint& Skeleton::joint(int bodypart)
{
	return joints[jointlabels[bodypart]];
}

Vector3& Skeleton::jointPos(int bodypart)
{
	return joint(bodypart).position;
}

Vector3& Skeleton::jointVel(int bodypart)
{
	return joint(bodypart).velocity;
}

/* EFFECT
 * sets forward, lowforward, specialforward[]
 *
 * USES:
 * Skeleton::Load
 * Person/Person::DoAnimations
 * Person/Person::DrawSkeleton
 */
void Skeleton::FindForwards()
{
	//Find forward vectors
	CrossProduct(joints[forwardjoints[1]].position - joints[forwardjoints[0]].position, joints[forwardjoints[2]].position - joints[forwardjoints[0]].position, &forward);
	Normalise(&forward);

	CrossProduct(joints[lowforwardjoints[1]].position - joints[lowforwardjoints[0]].position, joints[lowforwardjoints[2]].position - joints[lowforwardjoints[0]].position, &lowforward);
	Normalise(&lowforward);

	//Special forwards
	specialforward[0] = forward;

	specialforward[1] = jointPos(rightshoulder) + jointPos(rightwrist);
	specialforward[1] = jointPos(rightelbow) - specialforward[1] / 2.f;
	specialforward[1] += forward * .4f;
	Normalise(&specialforward[1]);
	specialforward[2] = jointPos(leftshoulder) + jointPos(leftwrist);
	specialforward[2] = jointPos(leftelbow) - specialforward[2] / 2.f;
	specialforward[2] += forward * .4f;
	Normalise(&specialforward[2]);

	specialforward[3] = jointPos(righthip) + jointPos(rightankle);
	specialforward[3] = specialforward[3] / 2.f - jointPos(rightknee);
	specialforward[3] += lowforward * .4f;
	Normalise(&specialforward[3]);
	specialforward[4] = jointPos(lefthip) + jointPos(leftankle);
	specialforward[4] = specialforward[4] / 2.f - jointPos(leftknee);
	specialforward[4] += lowforward * .4f;
	Normalise(&specialforward[4]);
}

/* EFFECT
 * TODO
 *
 * USES:
 * Person/Person::RagDoll
 * Person/Person::DoStuff
 * Person/IKHelper
 * 
 * Tutorial::active
 */
float Skeleton::DoConstraints(Vector3* coords, float* scale, bool tutorialActive, bool bloodtoggleflag, float timemultiplier, Terrain& terrainref, int envtype, float shakeamount, bool freeze, int detaillevel, int jointstartarray[26], int jointendarray[26])
{
	const float elasticity = .3f;
	Vector3 bounceness;
	const int numrepeats = 3;
	float groundlevel = .15f;
	unsigned i = 0;
	Vector3 temp;
	Vector3 terrainnormal;
	int whichhit = 0;
	float frictionness = 0.f;
	Vector3 terrainlight;
	int whichpatchx = 0;
	int whichpatchz = 0;
	float damage = 0; // eventually returned from function
	bool breaking = false;

	if (free) {
		freetime += timemultiplier;

		whichpatchx = terrainref.patchFor(coords->x);
		whichpatchz = terrainref.patchFor(coords->z);

		terrainlight = *coords;
		Object::SphereCheckPossible(&terrainlight, 1, terrainref);

		//Add velocity
		for (i = 0; i < joints.size(); i++) {
			joints[i].position = joints[i].position + joints[i].velocity * timemultiplier;

			switch (joints[i].label) {
			case head:
				groundlevel = .8f;
				break;
			case righthand:
			case rightwrist:
			case rightelbow:
			case lefthand:
			case leftwrist:
			case leftelbow:
				groundlevel = .2f;
				break;
			default:
				groundlevel = .15f;
				break;
			}

			joints[i].position.y -= groundlevel;
			joints[i].oldvelocity = joints[i].velocity;
		}

		float tempmult = timemultiplier;
		//multiplier/=numrepeats;

		for (int j = 0; j < numrepeats; j++) {
			float r = .05f;
			// right leg constraints?
			if (!joint(rightknee).locked && !joint(righthip).locked) {
				temp = jointPos(rightknee) - (jointPos(righthip) + jointPos(rightankle)) / 2;
				while (normaldotproduct(temp, lowforward) > -.1 && !sphere_line_intersection(&jointPos(righthip), &jointPos(rightankle), &jointPos(rightknee), &r)) {
					jointPos(rightknee) -= lowforward * .05f;
					if (spinny) {
						jointVel(rightknee) -= lowforward * .05f / timemultiplier / 4.f;
					}
					else {
						jointVel(rightknee) -= lowforward * .05f;
					}
					jointPos(rightankle) += lowforward * .025f;
					if (spinny) {
						jointVel(rightankle) += lowforward * .025f / timemultiplier / 4.f;
					}
					else {
						jointVel(rightankle) += lowforward * .25f;
					}
					jointPos(righthip) += lowforward * .025f;
					if (spinny) {
						jointVel(righthip) += lowforward * .025f / timemultiplier / 4.f;
					}
					else {
						jointVel(righthip) += lowforward * .025f;
					}
					temp = jointPos(rightknee) - (jointPos(righthip) + jointPos(rightankle)) / 2.f;
				}
			}

			// left leg constraints?
			if (!joint(leftknee).locked && !joint(lefthip).locked) {
				temp = jointPos(leftknee) - (jointPos(lefthip) + jointPos(leftankle)) / 2;
				while (normaldotproduct(temp, lowforward) > -.1 && !sphere_line_intersection(&jointPos(lefthip), &jointPos(leftankle), &jointPos(leftknee), &r)) {
					jointPos(leftknee) -= lowforward * .05f;
					if (spinny) {
						jointVel(leftknee) -= lowforward * .05f / timemultiplier / 4.f;
					}
					else {
						jointVel(leftknee) -= lowforward * .05f;
					}
					jointPos(leftankle) += lowforward * .025f;
					if (spinny) {
						jointVel(leftankle) += lowforward * .025f / timemultiplier / 4.f;
					}
					else {
						jointVel(leftankle) += lowforward * .25f;
					}
					jointPos(lefthip) += lowforward * .025f;
					if (spinny) {
						jointVel(lefthip) += lowforward * .025f / timemultiplier / 4.f;
					}
					else {
						jointVel(lefthip) += lowforward * .025f;
					}
					temp = jointPos(leftknee) - (jointPos(lefthip) + jointPos(leftankle)) / 2.f;
				}
			}

			for (i = 0; i < joints.size(); i++) {
				if (joints[i].locked && !spinny && magnitudeSquared(&joints[i].velocity) > 320.f) {
					joints[i].locked = 0;
				}
				if (spinny && magnitudeSquared(&joints[i].velocity) > 600.f) {
					joints[i].locked = 0;
				}
				if (joints[i].delay > 0) {
					bool freely = true;
					for (unsigned k = 0; k < joints.size(); k++) {
						if (joints[k].locked) {
							freely = false;
						}
					}
					if (freely) {
						joints[i].delay -= timemultiplier * 3.f;
					}
				}
			}

			for (i = 0; i < muscles.size(); i++) {
				//Length constraints
				muscles[i].DoConstraint(spinny, timemultiplier, freeze);
			}

			float friction;
			for (i = 0; i < joints.size(); i++) {
				//Length constraints
				//Ground constraint
				groundlevel = 0;
				if (joints[i].position.y * (*scale) + coords->y < terrainref.getHeight(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) + groundlevel) {
					freefall = 0;
					friction = 1.5;
					if (joints[i].label == groin && !joints[i].locked && joints[i].delay <= 0) {
						joints[i].locked = 1;
						joints[i].delay = 1;
						if (!tutorialActive || id == 0) {
							emit_sound_at(landsound1, joints[i].position * (*scale) + *coords, 128.f);
						}
						breaking = true;
					}

					if (joints[i].label == head && !joints[i].locked && joints[i].delay <= 0) {
						joints[i].locked = 1;
						joints[i].delay = 1;
						if (!tutorialActive || id == 0) {
							emit_sound_at(landsound2, joints[i].position * (*scale) + *coords, 128.f);
						}
					}

					terrainnormal = terrainref.getNormal(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z);
					ReflectVector(&joints[i].velocity, &terrainnormal);
					bounceness = terrainnormal * findLength(&joints[i].velocity) * (abs(normaldotproduct(joints[i].velocity, terrainnormal)));
					if (!joints[i].locked) {
						damage += magnitudeSquared(&bounceness) / 4000;
					}
					if (magnitudeSquared(&joints[i].velocity) < magnitudeSquared(&bounceness)) {
						bounceness = 0;
					}
					frictionness = abs(normaldotproduct(joints[i].velocity, terrainnormal));
					joints[i].velocity -= bounceness;
					if (1 - friction * frictionness > 0) {
						joints[i].velocity *= 1 - friction * frictionness;
					}
					else {
						joints[i].velocity = 0;
					}

					if (!tutorialActive || id == 0) {
						if (magnitudeSquared(&bounceness) > 8000 && breaking) {
							// FIXME: this crashes because k is not initialized!
							// to reproduce, type 'wolfie' in console and play a while
							// I'll just comment it out for now
							//Object::objects[k]->model.MakeDecal(breakdecal, DoRotation(temp - Object::objects[k]->position, 0, -Object::objects[k]->yaw, 0), .4, .5, rand() % 360);
							Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06f, 1, 1, 1, 4, .2f, bloodtoggleflag);
							breaking = false;
							shakeamount += .6f;

							emit_sound_at(breaksound2, joints[i].position * (*scale) + *coords);

							addEnvSound(*coords, 64);
						}
					}

					if (magnitudeSquared(&bounceness) > 2500) {
						Normalise(&bounceness);
						bounceness = bounceness * 50;
					}

					joints[i].velocity += bounceness * elasticity;

					if (magnitudeSquared(&joints[i].velocity) > magnitudeSquared(&joints[i].oldvelocity)) {
						bounceness = 0;
						joints[i].velocity = joints[i].oldvelocity;
					}

					if (joints[i].locked == 0) {
						if (magnitudeSquared(&joints[i].velocity) < 1) {
							joints[i].locked = 1;
						}
					}

					if (envtype == snowyenvironment && magnitudeSquared(&bounceness) > 500 && terrainref.getOpacity(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) < .2f) {
						terrainlight = terrainref.getLighting(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z);
						Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06f, terrainlight.x, terrainlight.y, terrainlight.z, .5f, .7f, bloodtoggleflag);
						if (detaillevel == 2) {
							terrainref.MakeDecal(bodyprintdecal, joints[i].position * (*scale) + *coords, .4f, .4f, 0, envtype);
						}
					}
					else if (envtype == desertenvironment && magnitudeSquared(&bounceness) > 500 && terrainref.getOpacity(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) < .2f) {
						terrainlight = terrainref.getLighting(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z);
						Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06f, terrainlight.x * 190 / 255, terrainlight.y * 170 / 255, terrainlight.z * 108 / 255, .5f, .7f, bloodtoggleflag);
					}

					else if (envtype == grassyenvironment && magnitudeSquared(&bounceness) > 500 && terrainref.getOpacity(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) < .2f) {
						terrainlight = terrainref.getLighting(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z);
						Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06f, terrainlight.x * 90 / 255, terrainlight.y * 70 / 255, terrainlight.z * 8 / 255, .5f, .5f, bloodtoggleflag);
					}
					else if (magnitudeSquared(&bounceness) > 500) {
						Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06f, terrainlight.x, terrainlight.y, terrainlight.z, .5f, .2f, bloodtoggleflag);
					}

					joints[i].position.y = (terrainref.getHeight(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) + groundlevel - coords->y) / (*scale);
					if (longdead > 100) {
						broken = 1;
					}
				}
				for (unsigned int m = 0; m < terrainref.patchobjects[whichpatchx][whichpatchz].size(); m++) {
					unsigned int k = terrainref.patchobjects[whichpatchx][whichpatchz][m];
					if (k < Object::objects.size()) {
						if (Object::objects[k]->possible) {
							friction = Object::objects[k]->friction;
							Vector3 start = joints[i].realoldposition;
							Vector3 end = joints[i].position * (*scale) + *coords;
							whichhit = Object::objects[k]->model.LineCheckPossible(&start, &end, &temp, &Object::objects[k]->position, &Object::objects[k]->yaw);
							if (whichhit != -1) {
								if (joints[i].label == groin && !joints[i].locked && joints[i].delay <= 0) {
									joints[i].locked = 1;
									joints[i].delay = 1;
									if (!tutorialActive || id == 0) {
										emit_sound_at(landsound1, joints[i].position * (*scale) + *coords, 128.);
									}
									breaking = true;
								}

								if (joints[i].label == head && !joints[i].locked && joints[i].delay <= 0) {
									joints[i].locked = 1;
									joints[i].delay = 1;
									if (!tutorialActive || id == 0) {
										emit_sound_at(landsound2, joints[i].position * (*scale) + *coords, 128.);
									}
								}

								terrainnormal = DoRotation(Object::objects[k]->model.Triangles[whichhit].facenormal, 0, Object::objects[k]->yaw, 0) * -1;
								if (terrainnormal.y > .8) {
									freefall = 0;
								}
								bounceness = terrainnormal * findLength(&joints[i].velocity) * (abs(normaldotproduct(joints[i].velocity, terrainnormal)));
								if (magnitudeSquared(&joints[i].velocity) > magnitudeSquared(&joints[i].oldvelocity)) {
									bounceness = 0;
									joints[i].velocity = joints[i].oldvelocity;
								}
								if (!tutorialActive || id == 0) {
									if (magnitudeSquared(&bounceness) > 4000 && breaking) {
										Object::objects[k]->model.MakeDecal(breakdecal, DoRotation(temp - Object::objects[k]->position, 0, -Object::objects[k]->yaw, 0), .4f, .5f, (float)(rand() % 360));
										Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06f, 1, 1, 1, 4, .2f, bloodtoggleflag);
										breaking = false;
										shakeamount += .6f;

										emit_sound_at(breaksound2, joints[i].position * (*scale) + *coords);

										addEnvSound(*coords, 64);
									}
								}
								if (Object::objects[k]->type == treetrunktype) {
									Object::objects[k]->rotx += joints[i].velocity.x * timemultiplier * .4f;
									Object::objects[k]->roty += joints[i].velocity.z * timemultiplier * .4f;
									Object::objects[k + 1]->rotx += joints[i].velocity.x * timemultiplier * .4f;
									Object::objects[k + 1]->roty += joints[i].velocity.z * timemultiplier * .4f;
								}
								if (!joints[i].locked) {
									damage += magnitudeSquared(&bounceness) / 2500;
								}
								ReflectVector(&joints[i].velocity, &terrainnormal);
								frictionness = abs(normaldotproduct(joints[i].velocity, terrainnormal));
								joints[i].velocity -= bounceness;
								if (1 - friction * frictionness > 0) {
									joints[i].velocity *= 1 - friction * frictionness;
								}
								else {
									joints[i].velocity = 0;
								}
								if (magnitudeSquared(&bounceness) > 2500) {
									Normalise(&bounceness);
									bounceness = bounceness * 50;
								}
								joints[i].velocity += bounceness * elasticity;

								if (!joints[i].locked) {
									if (magnitudeSquared(&joints[i].velocity) < 1) {
										joints[i].locked = 1;
									}
								}
								if (magnitudeSquared(&bounceness) > 500) {
									Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06f, 1, 1, 1, .5f, .2f, bloodtoggleflag);
								}
								joints[i].position = (temp - *coords) / (*scale) + terrainnormal * .005f;
								if (longdead > 100) {
									broken = 1;
								}
							}
						}
					}
				}
				joints[i].realoldposition = joints[i].position * (*scale) + *coords;
			}
		}
		timemultiplier = tempmult;

		for (unsigned int m = 0; m < terrainref.patchobjects[whichpatchx][whichpatchz].size(); m++) {
			unsigned int k = terrainref.patchobjects[whichpatchx][whichpatchz][m];
			if (Object::objects[k]->possible) {
				for (i = 0; i < 26; i++) {
					//Make this less stupid
					Vector3 start = joints[jointlabels[jointstartarray[i]]].position * (*scale) + *coords;
					Vector3 end = joints[jointlabels[jointendarray[i]]].position * (*scale) + *coords;
					whichhit = Object::objects[k]->model.LineCheckSlidePossible(&start, &end, &Object::objects[k]->position, &Object::objects[k]->yaw);
					if (whichhit != -1) {
						joints[jointlabels[jointendarray[i]]].position = (end - *coords) / (*scale);
						for (unsigned j = 0; j < muscles.size(); j++) {
							if ((muscles[j].parent1->label == jointstartarray[i] && muscles[j].parent2->label == jointendarray[i]) || (muscles[j].parent2->label == jointstartarray[i] && muscles[j].parent1->label == jointendarray[i])) {
								muscles[j].DoConstraint(spinny, timemultiplier, freeze);
							}
						}
					}
				}
			}
		}

		for (i = 0; i < joints.size(); i++) {
			switch (joints[i].label) {
			case head:
				groundlevel = .8f;
				break;
			case righthand:
			case rightwrist:
			case rightelbow:
			case lefthand:
			case leftwrist:
			case leftelbow:
				groundlevel = .2f;
				break;
			default:
				groundlevel = .15f;
				break;
			}
			joints[i].position.y += groundlevel;
			joints[i].mass = 1;
			if (joints[i].label == lefthip || joints[i].label == leftknee || joints[i].label == leftankle || joints[i].label == righthip || joints[i].label == rightknee || joints[i].label == rightankle) {
				joints[i].mass = 2;
			}
			if (joints[i].locked) {
				joints[i].mass = 4;
			}
		}

		return damage;
	}

	if (!free) {
		for (i = 0; i < muscles.size(); i++) {
			if (muscles[i].type == boneconnect) {
				muscles[i].DoConstraint(0, timemultiplier, freeze);
			}
		}
	}

	return 0;
}

/* EFFECT
 * applies gravity to the skeleton
 *
 * USES:
 * Person/Person::DoStuff
 */
void Skeleton::DoGravity(float* scale, float timemultiplier, float gravityamount)
{
	for (unsigned i = 0; i < joints.size(); i++) {
		if (
			(
				((joints[i].label != leftknee) && (joints[i].label != rightknee)) ||
				(lowforward.y > -.1) ||
				(joints[i].mass < 5)) &&
			(((joints[i].label != leftelbow) && (joints[i].label != rightelbow)) ||
				(forward.y < .3))) {
			joints[i].velocity.y += gravityamount * timemultiplier / (*scale);
		}
	}
}

/* EFFECT
 * set muscles[which].rotate1
 *     .rotate2
 *     .rotate3
 *
 * special case if animation == hanganim
 */
void Skeleton::FindRotationMuscle(int which, int animation)
{
	Vector3 p1, p2, fwd;
	float dist;

	p1 = muscles[which].parent1->position;
	p2 = muscles[which].parent2->position;
	dist = findDistance(&p1, &p2);
	if (p1.y - p2.y <= dist) {
		muscles[which].rotate2 = asin((p1.y - p2.y) / dist);
	}
	if (p1.y - p2.y > dist) {
		muscles[which].rotate2 = asin(1.f);
	}
	muscles[which].rotate2 *= 360.0f / 6.2831853f;

	p1.y = 0;
	p2.y = 0;
	dist = findDistance(&p1, &p2);
	if (p1.z - p2.z <= dist) {
		muscles[which].rotate1 = acos((p1.z - p2.z) / dist);
	}
	if (p1.z - p2.z > dist) {
		muscles[which].rotate1 = acos(1.f);
	}
	muscles[which].rotate1 *= 360.0f / 6.2831853f;
	if (p1.x > p2.x) {
		muscles[which].rotate1 = 360 - muscles[which].rotate1;
	}
	if (!isnormal(muscles[which].rotate1)) {
		muscles[which].rotate1 = 0;
	}
	if (!isnormal(muscles[which].rotate2)) {
		muscles[which].rotate2 = 0;
	}

	const int label1 = muscles[which].parent1->label;
	const int label2 = muscles[which].parent2->label;
	switch (label1) {
	case head:
		fwd = specialforward[0];
		break;
	case rightshoulder:
	case rightelbow:
	case rightwrist:
	case righthand:
		fwd = specialforward[1];
		break;
	case leftshoulder:
	case leftelbow:
	case leftwrist:
	case lefthand:
		fwd = specialforward[2];
		break;
	case righthip:
	case rightknee:
	case rightankle:
	case rightfoot:
		fwd = specialforward[3];
		break;
	case lefthip:
	case leftknee:
	case leftankle:
	case leftfoot:
		fwd = specialforward[4];
		break;
	default:
		if (muscles[which].parent1->lower) {
			fwd = lowforward;
		}
		else {
			fwd = forward;
		}
		break;
	}

	if (animation == hanganim) {
		if (label1 == righthand || label2 == righthand) {
			fwd = 0;
			fwd.x = -1;
		}
		if (label1 == lefthand || label2 == lefthand) {
			fwd = 0;
			fwd.x = 1;
		}
	}

	if (free == 0) {
		if (label1 == rightfoot || label2 == rightfoot) {
			fwd.y -= .3f;
		}
		if (label1 == leftfoot || label2 == leftfoot) {
			fwd.y -= .3f;
		}
	}

	fwd = DoRotation(fwd, 0, muscles[which].rotate1 - 90, 0);
	fwd = DoRotation(fwd, 0, 0, muscles[which].rotate2 - 90);
	fwd.y = 0;
	fwd /= findLength(&fwd);
	if (fwd.z <= 1 && fwd.z >= -1) {
		muscles[which].rotate3 = acos(0 - fwd.z);
	}
	else {
		muscles[which].rotate3 = acos(-1.f);
	}
	muscles[which].rotate3 *= 360.0f / 6.2831853f;
	if (0 > fwd.x) {
		muscles[which].rotate3 = 360 - muscles[which].rotate3;
	}
	if (!isnormal(muscles[which].rotate3)) {
		muscles[which].rotate3 = 0;
	}
}

namespace
{
// Figure data stores its rotations in degrees.
const float kDegreesToRadians = 0.017453292519943295f;

// One rotation about a unit axis, in the form glRotatef built.
void rotateAboutAxis(Vector3& point, float angle, float x, float y, float z)
{
	const float cosine = static_cast<float>(cos(angle * kDegreesToRadians));
	const float sine = static_cast<float>(sin(angle * kDegreesToRadians));
	const float oneMinusCosine = 1.0f - cosine;

	const float rotatedx = point.x * (x * x * oneMinusCosine + cosine) + point.y * (x * y * oneMinusCosine - z * sine) + point.z * (x * z * oneMinusCosine + y * sine);
	const float rotatedy = point.x * (y * x * oneMinusCosine + z * sine) + point.y * (y * y * oneMinusCosine + cosine) + point.z * (y * z * oneMinusCosine - x * sine);
	const float rotatedz = point.x * (z * x * oneMinusCosine - y * sine) + point.y * (z * y * oneMinusCosine + x * sine) + point.z * (z * z * oneMinusCosine + cosine);

	point.x = rotatedx;
	point.y = rotatedy;
	point.z = rotatedz;
}

// The three rotations of a muscle, in the order the matrix stack applied them.
// The stack read back the translation column, which is the point after all
// three rotations, so applying them to the point in reverse is the same thing.
Vector3 rotatePointByMuscle(Vector3 point, const Muscle& muscle)
{
	rotateAboutAxis(point, muscle.rotate1 - 90, 0, 1, 0);
	rotateAboutAxis(point, muscle.rotate2 - 90, 0, 0, 1);
	rotateAboutAxis(point, muscle.rotate3, 0, 1, 0);
	return point;
}

// The midpoint between the two joints a muscle spans, which the vertices are
// expressed relative to.
Vector3 muscleMidpoint(const Muscle& muscle)
{
	return (muscle.parent1->position + muscle.parent2->position) / 2;
}
}

/* EFFECT
 * load skeleton
 * takes filenames for three skeleton files and various models
 */
void Skeleton::Load(const std::string& filename, const std::string& lowfilename, const std::string& clothesfilename,
	const std::string& modelfilename, const std::string& model2filename,
	const std::string& model3filename, const std::string& model4filename,
	const std::string& model5filename, const std::string& model6filename,
	const std::string& model7filename, const std::string& modellowfilename,
	const std::string& modelclothesfilename, bool aclothes, bool tutorialActive, ProgressCallback callback)
{
	FILE* tfile = nullptr;
	size_t lSize = 0;
	int j, num_joints, num_muscles;

	num_models = 7;

	// load various models
	// rotate, scale, do normals, do texcoords for each as needed

	model[0].loadnotex(modelfilename);
	model[1].loadnotex(model2filename);
	model[2].loadnotex(model3filename);
	model[3].loadnotex(model4filename);
	model[4].loadnotex(model5filename);
	model[5].loadnotex(model6filename);
	model[6].loadnotex(model7filename);

	for (int i = 0; i < num_models; i++) {
		model[i].Rotate(180, 0, 0);
		model[i].Scale(.04f, .04f, .04f);
		model[i].CalculateNormals(0, callback);
	}

	drawmodel.load(modelfilename, callback);
	drawmodel.Rotate(180, 0, 0);
	drawmodel.Scale(.04f, .04f, .04f);
	drawmodel.FlipTexCoords();
	if ((tutorialActive) && (id != 0)) {
		drawmodel.UniformTexCoords();
		drawmodel.ScaleTexCoords(0.1);
	}
	drawmodel.CalculateNormals(0, callback);

	modellow.loadnotex(modellowfilename);
	modellow.Rotate(180, 0, 0);
	modellow.Scale(.04f, .04f, .04f);
	modellow.CalculateNormals(0, callback);

	drawmodellow.load(modellowfilename, callback);
	drawmodellow.Rotate(180, 0, 0);
	drawmodellow.Scale(.04f, .04f, .04f);
	drawmodellow.FlipTexCoords();
	if (tutorialActive && id != 0) {
		drawmodellow.UniformTexCoords();
	}
	if (tutorialActive && id != 0) {
		drawmodellow.ScaleTexCoords(0.1f);
	}
	drawmodellow.CalculateNormals(0, callback);

	if (aclothes) {
		modelclothes.loadnotex(modelclothesfilename);
		modelclothes.Rotate(180, 0, 0);
		modelclothes.Scale(.041f, .04f, .041f);
		modelclothes.CalculateNormals(0, callback);

		drawmodelclothes.load(modelclothesfilename, callback);
		drawmodelclothes.Rotate(180, 0, 0);
		drawmodelclothes.Scale(.04f, .04f, .04f);
		drawmodelclothes.FlipTexCoords();
		drawmodelclothes.CalculateNormals(0, callback);
	}

	// FIXME: three similar blocks follow, one for each of:
	// filename, lowfilename, clothesfilename

	// load skeleton

	tfile = Folders::openMandatoryFile(Folders::getResourcePath(filename), "rb");

	// read num_joints
	funpackf(tfile, "Bi", &num_joints);

	joints.clear();
	joints.resize(num_joints);

	// read info for each joint
	for (int i = 0; i < num_joints; i++) {
		joints[i].load(tfile, joints);
	}

	// read num_muscles
	funpackf(tfile, "Bi", &num_muscles);

	// allocate memory
	muscles.clear();
	muscles.resize(num_muscles);

	// for each muscle...
	for (int i = 0; i < num_muscles; i++) {
		muscles[i].load(tfile, model[0].vertexNum, joints);
	}

	// read forwardjoints (?)
	for (j = 0; j < 3; j++) {
		funpackf(tfile, "Bi", &forwardjoints[j]);
	}
	// read lowforwardjoints (?)
	for (j = 0; j < 3; j++) {
		funpackf(tfile, "Bi", &lowforwardjoints[j]);
	}

	// ???
	for (j = 0; j < num_muscles; j++) {
		for (unsigned i = 0; i < muscles[j].vertices.size(); i++) {
			for (int k = 0; k < num_models; k++) {
				if (muscles[j].vertices[i] < model[k].vertexNum) {
					model[k].owner[muscles[j].vertices[i]] = j;
				}
			}
		}
	}

	// calculate some stuff
	FindForwards();
	for (int i = 0; i < num_muscles; i++) {
		FindRotationMuscle(i, -1);
	}
	// this used to use opengl purely for matrix calculations
	for (int k = 0; k < num_models; k++) {
		for (int i = 0; i < model[k].vertexNum; i++) {
			model[k].vertex[i] = model[k].vertex[i] - muscleMidpoint(muscles[model[k].owner[i]]);
			model[k].vertex[i] = rotatePointByMuscle(model[k].vertex[i], muscles[model[k].owner[i]]);
		}
		model[k].CalculateNormals(0, callback);
	}
	fclose(tfile);

	// load ???

	tfile = Folders::openMandatoryFile(Folders::getResourcePath(lowfilename), "rb");

	// skip joints section

	fseek(tfile, sizeof(num_joints), SEEK_CUR);
	for (int i = 0; i < num_joints; i++) {
		// skip joint info
		lSize = sizeof(Vector3) + sizeof(float) + sizeof(float) + 1 //sizeof(bool)
			+ 1                                             //sizeof(bool)
			+ sizeof(int) + 1                               //sizeof(bool)
			+ 1                                             //sizeof(bool)
			+ sizeof(int) + sizeof(int) + 1                 //sizeof(bool)
			+ sizeof(int);
		fseek(tfile, lSize, SEEK_CUR);
	}

	// skip num_muscles
	fseek(tfile, sizeof(num_muscles), SEEK_CUR);

	for (int i = 0; i < num_muscles; i++) {
		// skip muscle info
		lSize = sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(int);
		fseek(tfile, lSize, SEEK_CUR);

		muscles[i].loadVerticesLow(tfile, modellow.vertexNum);

		// skip more stuff
		lSize = 1; //sizeof(bool);
		fseek(tfile, lSize, SEEK_CUR);
		lSize = sizeof(int);
		fseek(tfile, lSize, SEEK_CUR);
		fseek(tfile, lSize, SEEK_CUR);
	}

	for (j = 0; j < num_muscles; j++) {
		for (unsigned i = 0; i < muscles[j].verticeslow.size(); i++) {
			if (muscles[j].verticeslow[i] < modellow.vertexNum) {
				modellow.owner[muscles[j].verticeslow[i]] = j;
			}
		}
	}

	// the matrix stack was only used for matrix maths here too
	for (int i = 0; i < modellow.vertexNum; i++) {
		modellow.vertex[i] = modellow.vertex[i] - muscleMidpoint(muscles[modellow.owner[i]]);
		modellow.vertex[i] = rotatePointByMuscle(modellow.vertex[i], muscles[modellow.owner[i]]);
	}

	modellow.CalculateNormals(0, callback);

	// load clothes

	if (aclothes) {
		tfile = Folders::openMandatoryFile(Folders::getResourcePath(clothesfilename), "rb");

		// skip num_joints
		fseek(tfile, sizeof(num_joints), SEEK_CUR);

		for (int i = 0; i < num_joints; i++) {
			// skip joint info
			lSize = sizeof(Vector3) + sizeof(float) + sizeof(float) + 1 //sizeof(bool)
				+ 1                                             //sizeof(bool)
				+ sizeof(int) + 1                               //sizeof(bool)
				+ 1                                             //sizeof(bool)
				+ sizeof(int) + sizeof(int) + 1                 //sizeof(bool)
				+ sizeof(int);
			fseek(tfile, lSize, SEEK_CUR);
		}

		// skip num_muscles
		fseek(tfile, sizeof(num_muscles), SEEK_CUR);

		for (int i = 0; i < num_muscles; i++) {
			// skip muscle info
			lSize = sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(int);
			fseek(tfile, lSize, SEEK_CUR);

			muscles[i].loadVerticesClothes(tfile, modelclothes.vertexNum);

			// skip more stuff
			lSize = 1; //sizeof(bool);
			fseek(tfile, lSize, SEEK_CUR);
			lSize = sizeof(int);
			fseek(tfile, lSize, SEEK_CUR);
			fseek(tfile, lSize, SEEK_CUR);
		}

		// ???
		lSize = sizeof(int);
		for (j = 0; j < num_muscles; j++) {
			for (unsigned i = 0; i < muscles[j].verticesclothes.size(); i++) {
				if (muscles[j].verticesclothes.size() && muscles[j].verticesclothes[i] < modelclothes.vertexNum) {
					modelclothes.owner[muscles[j].verticesclothes[i]] = j;
				}
			}
		}

		// the matrix stack was only used for matrix maths here too
		for (int i = 0; i < modelclothes.vertexNum; i++) {
			modelclothes.vertex[i] = modelclothes.vertex[i] - muscleMidpoint(muscles[modelclothes.owner[i]]);
			modelclothes.vertex[i] = rotatePointByMuscle(modelclothes.vertex[i], muscles[modelclothes.owner[i]]);
		}

		modelclothes.CalculateNormals(0, callback);
	}
	fclose(tfile);

	for (int i = 0; i < num_joints; i++) {
		for (j = 0; j < num_joints; j++) {
			if (joints[i].label == j) {
				jointlabels[j] = i;
			}
		}
	}

	free = 0;
}
