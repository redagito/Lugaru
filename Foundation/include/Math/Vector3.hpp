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

#ifndef _QUATERNIONS_HPP_
#define _QUATERNIONS_HPP_

#include <cmath>
#include <json/value.h>

class Vector3
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	Vector3() = default;
	Vector3(float x, float y, float z);

	Vector3(Json::Value v);
	Vector3 operator+(const Vector3& add) const;
	Vector3 operator-(const Vector3& add) const;
	Vector3 operator*(float add) const;
	Vector3 operator*(const Vector3& add) const;
	Vector3 operator/(float add) const;
	void operator+=(const Vector3& add);
	void operator-=(const Vector3& add);
	void operator*=(float add);
	void operator*=(const Vector3& add);
	void operator/=(float add);
	void operator=(float add);
	bool operator==(const Vector3& add) const;

	operator Json::Value();
};

void CrossProduct(Vector3* P, Vector3* Q, Vector3* V);
void CrossProduct(Vector3 P, Vector3 Q, Vector3* V);
void Normalise(Vector3* vectory);
float normaldotproduct(Vector3 point1, Vector3 point2);
bool PointInTriangle(Vector3* p, Vector3 normal, Vector3* p1, Vector3* p2, Vector3* p3);

bool LineFacet(Vector3 p1, Vector3 p2, Vector3 pa, Vector3 pb, Vector3 pc, Vector3* p);
/** Hit test against a facet. Returns 1.0 on a hit, 0.0 otherwise; not a distance. */
float LineFacetHit(Vector3* p1, Vector3* p2, Vector3* pa, Vector3* pb, Vector3* pc, Vector3* n, Vector3* p);
float LineFacetHit(Vector3* p1, Vector3* p2, Vector3* pa, Vector3* pb, Vector3* pc, Vector3* p);

void ReflectVector(Vector3* vel, const Vector3* n);
void ReflectVector(Vector3* vel, const Vector3& n);
Vector3 DoRotation(Vector3 thePoint, float xang, float yang, float zang);
Vector3 DoRotationRadian(Vector3 thePoint, float xang, float yang, float zang);

float findDistance(const Vector3* point1, const Vector3* point2);
float findLength(const Vector3* point1);
/** Squared magnitude (no sqrt). Every threshold in the engine is calibrated against this, so do not change the return value. */
float magnitudeSquared(const Vector3* point1);

float distsq(const Vector3* point1, const Vector3* point2);
float distsq(const Vector3& point1, const Vector3& point2);
float distsqflat(const Vector3* point1, const Vector3* point2);
float dotproduct(const Vector3* point1, const Vector3* point2);

bool sphere_line_intersection(
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3, float r);

bool sphere_line_intersection(
	Vector3* p1, Vector3* p2, Vector3* p3, float* r);
bool DistancePointLine(Vector3* Point, Vector3* LineStart, Vector3* LineEnd, float* Distance, Vector3* Intersection);

/** Square of a value. Its own function because the name reads better at the threshold sites. */
float square(float f);

#endif
