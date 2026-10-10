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
#include "Math/Vector3.hpp"

bool PointInTriangle(Vector3* p, Vector3 normal, Vector3* p1, Vector3* p2, Vector3* p3)
{
    float u0, u1, u2;
    float v0, v1, v2;
    float a, b;
    float max;
    // Seeded to 0 to match the old statics on the first call. On later calls the
    // old statics kept stale axis indices from the previous call; the old warm-call
    // behaviour was history-dependent, so this is strictly more deterministic.
    int i = 0, j = 0;
    bool bInter = 0;
    float pointv[3];
    float p1v[3];
    float p2v[3];
    float p3v[3];
    float normalv[3];

    bInter = 0;

    pointv[0] = p->x;
    pointv[1] = p->y;
    pointv[2] = p->z;

    p1v[0] = p1->x;
    p1v[1] = p1->y;
    p1v[2] = p1->z;

    p2v[0] = p2->x;
    p2v[1] = p2->y;
    p2v[2] = p2->z;

    p3v[0] = p3->x;
    p3v[1] = p3->y;
    p3v[2] = p3->z;

    normalv[0] = normal.x;
    normalv[1] = normal.y;
    normalv[2] = normal.z;

#define ABS(X) (((X) < 0.f) ? -(X) : (X))
#define MAX(A, B) (((A) < (B)) ? (B) : (A))
    max = MAX(MAX(ABS(normalv[0]), ABS(normalv[1])), ABS(normalv[2]));
#undef MAX
    if (max == ABS(normalv[0])) {
        i = 1; // y, z
        j = 2;
    }
    if (max == ABS(normalv[1])) {
        i = 0; // x, z
        j = 2;
    }
    if (max == ABS(normalv[2])) {
        i = 0; // x, y
        j = 1;
    }
#undef ABS

    u0 = pointv[i] - p1v[i];
    v0 = pointv[j] - p1v[j];
    u1 = p2v[i] - p1v[i];
    v1 = p2v[j] - p1v[j];
    u2 = p3v[i] - p1v[i];
    v2 = p3v[j] - p1v[j];

    if (u1 > -1.0e-05f && u1 < 1.0e-05f) { // == 0.0f)
        b = u0 / u2;
        if (0.0f <= b && b <= 1.0f) {
            a = (v0 - b * v2) / v1;
            if ((a >= 0.0f) && ((a + b) <= 1.0f)) {
                bInter = 1;
            }
        }
    } else {
        b = (v0 * u1 - u0 * v1) / (v2 * u1 - u2 * v1);
        if (0.0f <= b && b <= 1.0f) {
            a = (u0 - b * u2) / u1;
            if ((a >= 0.0f) && ((a + b) <= 1.0f)) {
                bInter = 1;
            }
        }
    }

    return bInter;
}

bool LineFacet(Vector3 p1, Vector3 p2, Vector3 pa, Vector3 pb, Vector3 pc, Vector3* p)
{
    float d;
    float denom, mu;
    Vector3 n;

    //Calculate the parameters for the plane
    n.x = (pb.y - pa.y) * (pc.z - pa.z) - (pb.z - pa.z) * (pc.y - pa.y);
    n.y = (pb.z - pa.z) * (pc.x - pa.x) - (pb.x - pa.x) * (pc.z - pa.z);
    n.z = (pb.x - pa.x) * (pc.y - pa.y) - (pb.y - pa.y) * (pc.x - pa.x);
    Normalise(&n);
    d = -n.x * pa.x - n.y * pa.y - n.z * pa.z;

    //Calculate the position on the line that intersects the plane
    denom = n.x * (p2.x - p1.x) + n.y * (p2.y - p1.y) + n.z * (p2.z - p1.z);
    if (fabs(denom) < 0.0000001) { // Line and plane don't intersect
        return 0;
    }
    mu = -(d + n.x * p1.x + n.y * p1.y + n.z * p1.z) / denom;
    p->x = p1.x + mu * (p2.x - p1.x);
    p->y = p1.y + mu * (p2.y - p1.y);
    p->z = p1.z + mu * (p2.z - p1.z);
    if (mu < 0 || mu > 1) { // Intersection not along line segment
        return 0;
    }

    if (!PointInTriangle(p, n, &pa, &pb, &pc)) {
        return 0;
    }

    return 1;
}


float LineFacetHit(Vector3* p1, Vector3* p2, Vector3* pa, Vector3* pb, Vector3* pc, Vector3* p)
{
    float d;
    float denom, mu;
    Vector3 n;

    //Calculate the parameters for the plane
    n.x = (pb->y - pa->y) * (pc->z - pa->z) - (pb->z - pa->z) * (pc->y - pa->y);
    n.y = (pb->z - pa->z) * (pc->x - pa->x) - (pb->x - pa->x) * (pc->z - pa->z);
    n.z = (pb->x - pa->x) * (pc->y - pa->y) - (pb->y - pa->y) * (pc->x - pa->x);
    Normalise(&n);
    d = -n.x * pa->x - n.y * pa->y - n.z * pa->z;

    //Calculate the position on the line that intersects the plane
    denom = n.x * (p2->x - p1->x) + n.y * (p2->y - p1->y) + n.z * (p2->z - p1->z);
    if (fabs(denom) < 0.0000001) { // Line and plane don't intersect
        return 0;
    }
    mu = -(d + n.x * p1->x + n.y * p1->y + n.z * p1->z) / denom;
    p->x = p1->x + mu * (p2->x - p1->x);
    p->y = p1->y + mu * (p2->y - p1->y);
    p->z = p1->z + mu * (p2->z - p1->z);
    if (mu < 0 || mu > 1) { // Intersection not along line segment
        return 0;
    }

    if (!PointInTriangle(p, n, pa, pb, pc)) {
        return 0;
    }
    return 1;
}

float LineFacetHit(Vector3* p1, Vector3* p2, Vector3* pa, Vector3* pb, Vector3* pc, Vector3* n, Vector3* p)
{
    float d;
    float denom, mu;

    //Calculate the parameters for the plane
    d = -n->x * pa->x - n->y * pa->y - n->z * pa->z;

    //Calculate the position on the line that intersects the plane
    denom = n->x * (p2->x - p1->x) + n->y * (p2->y - p1->y) + n->z * (p2->z - p1->z);
    if (fabs(denom) < 0.0000001) { // Line and plane don't intersect
        return 0;
    }
    mu = -(d + n->x * p1->x + n->y * p1->y + n->z * p1->z) / denom;
    p->x = p1->x + mu * (p2->x - p1->x);
    p->y = p1->y + mu * (p2->y - p1->y);
    p->z = p1->z + mu * (p2->z - p1->z);
    if (mu < 0 || mu > 1) { // Intersection not along line segment
        return 0;
    }

    if (!PointInTriangle(p, *n, pa, pb, pc)) {
        return 0;
    }
    return 1;
}

Vector3::operator Json::Value()
{
    Json::Value xyz;

    xyz[0] = x;
    xyz[1] = y;
    xyz[2] = z;

    return xyz;
}

Vector3::Vector3(float x, float y, float z)
    : x(x)
    , y(y)
    , z(z)
{
}

Vector3::Vector3(Json::Value v)
    : x(v[0].asFloat())
    , y(v[1].asFloat())
    , z(v[2].asFloat())
{
}

void Normalise(Vector3* vectory)
{
    float d;
    d = sqrt(vectory->x * vectory->x + vectory->y * vectory->y + vectory->z * vectory->z);
    if (d == 0) {
        return;
    }
    vectory->x /= d;
    vectory->y /= d;
    vectory->z /= d;
}

Vector3 Vector3::operator+(const Vector3& add) const
{
    Vector3 ne;
    ne = add;
    ne.x += x;
    ne.y += y;
    ne.z += z;
    return ne;
}

Vector3 Vector3::operator-(const Vector3& add) const
{
    Vector3 ne;
    ne = add;
    ne.x = x - ne.x;
    ne.y = y - ne.y;
    ne.z = z - ne.z;
    return ne;
}

Vector3 Vector3::operator*(float add) const
{
    Vector3 ne;
    ne.x = x * add;
    ne.y = y * add;
    ne.z = z * add;
    return ne;
}

Vector3 Vector3::operator*(const Vector3& add) const
{
    Vector3 ne;
    ne.x = x * add.x;
    ne.y = y * add.y;
    ne.z = z * add.z;
    return ne;
}

Vector3 Vector3::operator/(float add) const
{
    Vector3 ne;
    ne.x = x / add;
    ne.y = y / add;
    ne.z = z / add;
    return ne;
}

void Vector3::operator+=(const Vector3& add)
{
    x += add.x;
    y += add.y;
    z += add.z;
}

void Vector3::operator-=(const Vector3& add)
{
    x = x - add.x;
    y = y - add.y;
    z = z - add.z;
}

void Vector3::operator*=(float add)
{
    x = x * add;
    y = y * add;
    z = z * add;
}

void Vector3::operator*=(const Vector3& add)
{
    x = x * add.x;
    y = y * add.y;
    z = z * add.z;
}

void Vector3::operator/=(float add)
{
    x = x / add;
    y = y / add;
    z = z / add;
}

void Vector3::operator=(float add)
{
    x = add;
    y = add;
    z = add;
}

bool Vector3::operator==(const Vector3& add) const
{
    if (x == add.x && y == add.y && z == add.z)
        return 1;
    return 0;
}

void CrossProduct(Vector3* P, Vector3* Q, Vector3* V)
{
    V->x = P->y * Q->z - P->z * Q->y;
    V->y = P->z * Q->x - P->x * Q->z;
    V->z = P->x * Q->y - P->y * Q->x;
}

void CrossProduct(Vector3 P, Vector3 Q, Vector3* V)
{
    V->x = P.y * Q.z - P.z * Q.y;
    V->y = P.z * Q.x - P.x * Q.z;
    V->z = P.x * Q.y - P.y * Q.x;
}

float normaldotproduct(Vector3 point1, Vector3 point2)
{
    float returnvalue;
    Normalise(&point1);
    Normalise(&point2);
    returnvalue = (point1.x * point2.x + point1.y * point2.y + point1.z * point2.z);
    return returnvalue;
}

void ReflectVector(Vector3* vel, const Vector3* n)
{
    ReflectVector(vel, *n);
}

void ReflectVector(Vector3* vel, const Vector3& n)
{
    Vector3 vn;
    Vector3 vt;
    float dotprod;

    dotprod = dotproduct(&n, vel);
    vn.x = n.x * dotprod;
    vn.y = n.y * dotprod;
    vn.z = n.z * dotprod;

    vt.x = vel->x - vn.x;
    vt.y = vel->y - vn.y;
    vt.z = vel->z - vn.z;

    vel->x = vt.x - vn.x;
    vel->y = vt.y - vn.y;
    vel->z = vt.z - vn.z;
}

float dotproduct(const Vector3* point1, const Vector3* point2)
{
    float returnvalue;
    returnvalue = (point1->x * point2->x + point1->y * point2->y + point1->z * point2->z);
    return returnvalue;
}

float findDistance(const Vector3* point1, const Vector3* point2)
{
    return (sqrt((point1->x - point2->x) * (point1->x - point2->x) + (point1->y - point2->y) * (point1->y - point2->y) + (point1->z - point2->z) * (point1->z - point2->z)));
}

float findLength(const Vector3* point1)
{
    return (sqrt((point1->x) * (point1->x) + (point1->y) * (point1->y) + (point1->z) * (point1->z)));
}

float magnitudeSquared(const Vector3* point1)
{
    return ((point1->x) * (point1->x) + (point1->y) * (point1->y) + (point1->z) * (point1->z));
}

float distsq(const Vector3* point1, const Vector3* point2)
{
    return ((point1->x - point2->x) * (point1->x - point2->x) + (point1->y - point2->y) * (point1->y - point2->y) + (point1->z - point2->z) * (point1->z - point2->z));
}

float distsq(const Vector3& point1, const Vector3& point2)
{
    return ((point1.x - point2.x) * (point1.x - point2.x) + (point1.y - point2.y) * (point1.y - point2.y) + (point1.z - point2.z) * (point1.z - point2.z));
}

float distsqflat(const Vector3* point1, const Vector3* point2)
{
    return ((point1->x - point2->x) * (point1->x - point2->x) + (point1->z - point2->z) * (point1->z - point2->z));
}

Vector3 DoRotation(Vector3 thePoint, float xang, float yang, float zang)
{
    Vector3 newpoint;
    if (xang) {
        xang *= 6.283185f;
        xang /= 360;
    }
    if (yang) {
        yang *= 6.283185f;
        yang /= 360;
    }
    if (zang) {
        zang *= 6.283185f;
        zang /= 360;
    }

    if (yang) {
        newpoint.z = thePoint.z * cosf(yang) - thePoint.x * sinf(yang);
        newpoint.x = thePoint.z * sinf(yang) + thePoint.x * cosf(yang);
        thePoint.z = newpoint.z;
        thePoint.x = newpoint.x;
    }

    if (zang) {
        newpoint.x = thePoint.x * cosf(zang) - thePoint.y * sinf(zang);
        newpoint.y = thePoint.y * cosf(zang) + thePoint.x * sinf(zang);
        thePoint.x = newpoint.x;
        thePoint.y = newpoint.y;
    }

    if (xang) {
        newpoint.y = thePoint.y * cosf(xang) - thePoint.z * sinf(xang);
        newpoint.z = thePoint.y * sinf(xang) + thePoint.z * cosf(xang);
        thePoint.z = newpoint.z;
        thePoint.y = newpoint.y;
    }

    return thePoint;
}

float square(float f)
{
    return (f * f);
}

bool sphere_line_intersection(
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    float x3, float y3, float z3, float r)
{
    // x1,y1,z1  P1 coordinates (point of line)
    // x2,y2,z2  P2 coordinates (point of line)
    // x3,y3,z3, r  P3 coordinates and radius (sphere)
    float a, b, c, i;

    if (x1 > x3 + r && x2 > x3 + r)
        return (0);
    if (x1 < x3 - r && x2 < x3 - r)
        return (0);
    if (y1 > y3 + r && y2 > y3 + r)
        return (0);
    if (y1 < y3 - r && y2 < y3 - r)
        return (0);
    if (z1 > z3 + r && z2 > z3 + r)
        return (0);
    if (z1 < z3 - r && z2 < z3 - r)
        return (0);
    a = square(x2 - x1) + square(y2 - y1) + square(z2 - z1);
    b = 2 * ((x2 - x1) * (x1 - x3) + (y2 - y1) * (y1 - y3) + (z2 - z1) * (z1 - z3));
    c = square(x3) + square(y3) +
        square(z3) + square(x1) +
        square(y1) + square(z1) -
        2 * (x3 * x1 + y3 * y1 + z3 * z1) - square(r);
    i = b * b - 4 * a * c;

    if (i < 0.0) {
        // no intersection
        return (0);
    }
    return (1);
}

bool sphere_line_intersection(
    Vector3* p1, Vector3* p2, Vector3* p3, float* r)
{
    float a, b, c, i;

    if (p1->x > p3->x + *r && p2->x > p3->x + *r)
        return (0);
    if (p1->x < p3->x - *r && p2->x < p3->x - *r)
        return (0);
    if (p1->y > p3->y + *r && p2->y > p3->y + *r)
        return (0);
    if (p1->y < p3->y - *r && p2->y < p3->y - *r)
        return (0);
    if (p1->z > p3->z + *r && p2->z > p3->z + *r)
        return (0);
    if (p1->z < p3->z - *r && p2->z < p3->z - *r)
        return (0);
    a = square(p2->x - p1->x) + square(p2->y - p1->y) + square(p2->z - p1->z);
    b = 2 * ((p2->x - p1->x) * (p1->x - p3->x) + (p2->y - p1->y) * (p1->y - p3->y) + (p2->z - p1->z) * (p1->z - p3->z));
    c = square(p3->x) + square(p3->y) +
        square(p3->z) + square(p1->x) +
        square(p1->y) + square(p1->z) -
        2 * (p3->x * p1->x + p3->y * p1->y + p3->z * p1->z) - square(*r);
    i = b * b - 4 * a * c;

    if (i < 0.0) {
        // no intersection
        return (0);
    }
    return (1);
}

Vector3 DoRotationRadian(Vector3 thePoint, float xang, float yang, float zang)
{
    Vector3 newpoint;
    Vector3 oldpoint;

    oldpoint = thePoint;

    if (yang != 0) {
        newpoint.z = oldpoint.z * cosf(yang) - oldpoint.x * sinf(yang);
        newpoint.x = oldpoint.z * sinf(yang) + oldpoint.x * cosf(yang);
        oldpoint.z = newpoint.z;
        oldpoint.x = newpoint.x;
    }

    if (zang != 0) {
        newpoint.x = oldpoint.x * cosf(zang) - oldpoint.y * sinf(zang);
        newpoint.y = oldpoint.y * cosf(zang) + oldpoint.x * sinf(zang);
        oldpoint.x = newpoint.x;
        oldpoint.y = newpoint.y;
    }

    if (xang != 0) {
        newpoint.y = oldpoint.y * cosf(xang) - oldpoint.z * sinf(xang);
        newpoint.z = oldpoint.y * sinf(xang) + oldpoint.z * cosf(xang);
        oldpoint.z = newpoint.z;
        oldpoint.y = newpoint.y;
    }

    return oldpoint;
}

bool DistancePointLine(Vector3* Point, Vector3* LineStart, Vector3* LineEnd, float* Distance, Vector3* Intersection)
{
    float LineMag;
    float U;

    LineMag = findDistance(LineEnd, LineStart);

    U = (((Point->x - LineStart->x) * (LineEnd->x - LineStart->x)) +
        ((Point->y - LineStart->y) * (LineEnd->y - LineStart->y)) +
        ((Point->z - LineStart->z) * (LineEnd->z - LineStart->z))) /
        (LineMag * LineMag);

    if (U < 0.0f || U > 1.0f)
        return 0; // closest point does not fall within the line segment

    Intersection->x = LineStart->x + U * (LineEnd->x - LineStart->x);
    Intersection->y = LineStart->y + U * (LineEnd->y - LineStart->y);
    Intersection->z = LineStart->z + U * (LineEnd->z - LineStart->z);

    *Distance = findDistance(Point, Intersection);

    return 1;
}
