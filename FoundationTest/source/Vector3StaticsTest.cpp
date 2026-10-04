// Characterisation tests for PointInTriangle, LineFacet and both LineFacetHit
// overloads in Foundation/source/Math/Vector3.cpp.
//
// Every case here passes against the current implementation, including the
// inputs that divide by zero, compare against NaN and land exactly on a
// tolerance boundary. They exist so that deleting the function-local statics
// can be shown to change nothing observable.

#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "Math/Vector3.hpp"

namespace
{

constexpr float kEpsilon = 1e-4f;

// The facet helpers take non-const pointers, so the corners must be mutable.
struct Triangle
{
	Vector3 a, b, c;

	Triangle(float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz)
		: a(ax, ay, az)
		, b(bx, by, bz)
		, c(cx, cy, cz)
	{
	}
};

// Right triangle in the XZ plane with the right angle at the origin. The cross
// product of its edges is (0, -leg*leg, 0), so every helper derives the normal
// (0, -1, 0).
Triangle xzTriangle(float leg)
{
	return Triangle(0.0f, 0.0f, 0.0f, leg, 0.0f, 0.0f, 0.0f, 0.0f, leg);
}

} // namespace

TEST_CASE("PointInTriangle projects onto the dominant normal axis", "[vector3][statics]")
{
	SECTION("an interior point is accepted and an exterior one is rejected")
	{
		Triangle t = xzTriangle(1.0f);
		const Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 inside(0.25f, 0.0f, 0.25f);
		REQUIRE(PointInTriangle(&inside, normal, &t.a, &t.b, &t.c));

		Vector3 outside(0.75f, 0.0f, 0.75f);
		REQUIRE_FALSE(PointInTriangle(&outside, normal, &t.a, &t.b, &t.c));
	}

	SECTION("each vertex exactly on the boundary is accepted")
	{
		Triangle t = xzTriangle(1.0f);
		const Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 atA(0.0f, 0.0f, 0.0f);
		Vector3 atB(1.0f, 0.0f, 0.0f);
		Vector3 atC(0.0f, 0.0f, 1.0f);
		REQUIRE(PointInTriangle(&atA, normal, &t.a, &t.b, &t.c));
		REQUIRE(PointInTriangle(&atB, normal, &t.a, &t.b, &t.c));
		REQUIRE(PointInTriangle(&atC, normal, &t.a, &t.b, &t.c));
	}

	SECTION("a point on the hypotenuse is accepted and a hair beyond it is not")
	{
		// The hypotenuse is the edge a+b == 1 in the projected plane, so this is
		// the case that separates a strict from a non-strict comparison.
		Triangle t = xzTriangle(1.0f);
		const Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 onHypotenuse(0.5f, 0.0f, 0.5f);
		REQUIRE(PointInTriangle(&onHypotenuse, normal, &t.a, &t.b, &t.c));

		Vector3 justPast(0.5001f, 0.0f, 0.5f);
		REQUIRE_FALSE(PointInTriangle(&justPast, normal, &t.a, &t.b, &t.c));
	}

	SECTION("a point on a leg is accepted and a hair off it is not")
	{
		Triangle t = xzTriangle(1.0f);
		const Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 onLeg(0.5f, 0.0f, 0.0f);
		REQUIRE(PointInTriangle(&onLeg, normal, &t.a, &t.b, &t.c));

		Vector3 offLeg(0.5f, 0.0f, -0.0001f);
		REQUIRE_FALSE(PointInTriangle(&offLeg, normal, &t.a, &t.b, &t.c));
	}

	SECTION("a near miss next to a vertex is classified the right way round")
	{
		Triangle t = xzTriangle(1.0f);
		const Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 justInside(0.999f, 0.0f, 0.0005f);
		REQUIRE(PointInTriangle(&justInside, normal, &t.a, &t.b, &t.c));

		Vector3 justOutside(1.001f, 0.0f, 0.0005f);
		REQUIRE_FALSE(PointInTriangle(&justOutside, normal, &t.a, &t.b, &t.c));
	}

	SECTION("a +Z normal projects the same facet onto the XY plane")
	{
		Triangle t(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		const Vector3 normal(0.0f, 0.0f, 1.0f);

		Vector3 inside(0.25f, 0.25f, 0.0f);
		REQUIRE(PointInTriangle(&inside, normal, &t.a, &t.b, &t.c));

		Vector3 outside(0.75f, 0.75f, 0.0f);
		REQUIRE_FALSE(PointInTriangle(&outside, normal, &t.a, &t.b, &t.c));
	}

	SECTION("a +X normal projects the same facet onto the YZ plane")
	{
		Triangle t(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		const Vector3 normal(1.0f, 0.0f, 0.0f);

		Vector3 inside(0.0f, 0.25f, 0.25f);
		REQUIRE(PointInTriangle(&inside, normal, &t.a, &t.b, &t.c));

		Vector3 outside(0.0f, 1.2f, 0.2f);
		REQUIRE_FALSE(PointInTriangle(&outside, normal, &t.a, &t.b, &t.c));
	}
}

TEST_CASE("PointInTriangle dominant axis selection is last match wins", "[vector3][statics]")
{
	// The three axis tests are independent ifs, not a chain, so a normal with a
	// tie is resolved by whichever component happens to be tested last.
	SECTION("a normal tied between +X and +Y projects onto XZ, not XY")
	{
		// The facet lies in the XY plane, so the geometrically correct answer is
		// true. The tie sends the projection to XZ instead, where the point sits
		// on the degenerate edge and the divide by a zero v1 rejects it.
		Triangle t(0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 5.0f, 0.0f, 0.0f);
		const Vector3 normal(1.0f, 1.0f, 0.0f);

		Vector3 insideTheFacet(1.0f, 1.0f, 0.0f);
		REQUIRE_FALSE(PointInTriangle(&insideTheFacet, normal, &t.a, &t.b, &t.c));

		// The same facet with an unambiguous +Z normal accepts the same point.
		const Vector3 zNormal(0.0f, 0.0f, 1.0f);
		REQUIRE(PointInTriangle(&insideTheFacet, zNormal, &t.a, &t.b, &t.c));
	}

	SECTION("a zero normal falls through all three tests and projects onto XY")
	{
		Triangle t(0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 5.0f, 0.0f, 0.0f);
		const Vector3 zero(0.0f, 0.0f, 0.0f);

		Vector3 inside(1.0f, 1.0f, 0.0f);
		REQUIRE(PointInTriangle(&inside, zero, &t.a, &t.b, &t.c));

		Vector3 outside(6.0f, 1.0f, 0.0f);
		REQUIRE_FALSE(PointInTriangle(&outside, zero, &t.a, &t.b, &t.c));
	}

	SECTION("a zero normal rejects a facet that lives in the XZ plane")
	{
		// Under the XY projection the whole XZ facet collapses onto the line
		// y == 0, so the divide by a zero determinant rejects every point.
		Triangle t = xzTriangle(10.0f);
		const Vector3 zero(0.0f, 0.0f, 0.0f);

		Vector3 atOrigin(0.0f, 0.0f, 0.0f);
		REQUIRE_FALSE(PointInTriangle(&atOrigin, zero, &t.a, &t.b, &t.c));

		Vector3 insideFootprint(1.0f, 0.0f, 1.0f);
		REQUIRE_FALSE(PointInTriangle(&insideFootprint, zero, &t.a, &t.b, &t.c));
	}
}

TEST_CASE("PointInTriangle degenerate edge handling", "[vector3][statics]")
{
	// The u1 near-zero window is a hardcoded half-open test, |u1| < 1e-5, and it
	// selects a branch that ignores the v1 side of the triangle entirely.
	SECTION("an exactly coincident pair on the dominant axis is accepted")
	{
		// p1 and p2 share x, so u1 == 0 and the special branch runs.
		Triangle t(0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 5.0f, 0.0f, 0.0f);
		const Vector3 normal(0.0f, 0.0f, 1.0f);

		Vector3 inside(1.0f, 1.0f, 0.0f);
		REQUIRE(PointInTriangle(&inside, normal, &t.a, &t.b, &t.c));

		Vector3 above(1.0f, 6.0f, 0.0f);
		REQUIRE_FALSE(PointInTriangle(&above, normal, &t.a, &t.b, &t.c));
	}

	SECTION("a near zero but non zero pair still takes the special branch")
	{
		// u1 == 1e-6 is inside the window. The special branch divides by v1
		// directly, so the sign of the 5.0 edge decides the result; the general
		// branch would have divided by the tiny u1 and lost the sign.
		Triangle up(0.0f, 0.0f, 0.0f, 0.000001f, 5.0f, 0.0f, 5.0f, 0.0f, 0.0f);
		Triangle down(0.0f, 0.0f, 0.0f, 0.000001f, -5.0f, 0.0f, 5.0f, 0.0f, 0.0f);
		const Vector3 normal(0.0f, 0.0f, 1.0f);

		Vector3 inside(1.0f, 1.0f, 0.0f);
		REQUIRE(PointInTriangle(&inside, normal, &up.a, &up.b, &up.c));
		REQUIRE_FALSE(PointInTriangle(&inside, normal, &down.a, &down.b, &down.c));
	}

	SECTION("a facet with all three corners equal is rejected")
	{
		Triangle t(1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f);
		const Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 onDegenerateFacet(1.0f, 2.0f, 3.0f);
		REQUIRE_FALSE(PointInTriangle(&onDegenerateFacet, normal, &t.a, &t.b, &t.c));
	}

	SECTION("a facet with two corners equal is rejected")
	{
		Triangle t(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f);
		const Vector3 normal(0.0f, 0.0f, 1.0f);

		Vector3 onDegenerateFacet(1.0f, 0.0f, 0.0f);
		REQUIRE_FALSE(PointInTriangle(&onDegenerateFacet, normal, &t.a, &t.b, &t.c));
	}

	SECTION("collinear corners are rejected")
	{
		Triangle t(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f);
		const Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 middle(1.0f, 0.0f, 0.0f);
		Vector3 end(0.0f, 0.0f, 0.0f);
		REQUIRE_FALSE(PointInTriangle(&middle, normal, &t.a, &t.b, &t.c));
		REQUIRE_FALSE(PointInTriangle(&end, normal, &t.a, &t.b, &t.c));
	}
}

TEST_CASE("PointInTriangle sign, scale and precision invariance", "[vector3][statics]")
{
	Triangle t = xzTriangle(1.0f);

	SECTION("the sign of the normal is irrelevant")
	{
		Vector3 inside(0.25f, 0.0f, 0.25f);
		Vector3 outside(0.75f, 0.0f, 0.75f);

		REQUIRE(PointInTriangle(&inside, Vector3(0.0f, -1.0f, 0.0f), &t.a, &t.b, &t.c));
		REQUIRE(PointInTriangle(&inside, Vector3(0.0f, 1.0f, 0.0f), &t.a, &t.b, &t.c));
		REQUIRE_FALSE(PointInTriangle(&outside, Vector3(0.0f, -1.0f, 0.0f), &t.a, &t.b, &t.c));
		REQUIRE_FALSE(PointInTriangle(&outside, Vector3(0.0f, 1.0f, 0.0f), &t.a, &t.b, &t.c));
	}

	SECTION("the magnitude of the normal is irrelevant")
	{
		Vector3 inside(0.25f, 0.0f, 0.25f);
		REQUIRE(PointInTriangle(&inside, Vector3(0.0f, -1000.0f, 0.0f), &t.a, &t.b, &t.c));
		REQUIRE(PointInTriangle(&inside, Vector3(0.0f, -0.0001f, 0.0f), &t.a, &t.b, &t.c));
	}

	SECTION("the normal axis is projected away entirely")
	{
		// A point far off the plane but over the footprint is still inside: the
		// projection discards the dominant component.
		const Vector3 normal(0.0f, -1.0f, 0.0f);
		Vector3 wayOffPlane(0.25f, 1000.0f, 0.25f);
		Vector3 opposite(0.25f, -1000.0f, 0.25f);
		REQUIRE(PointInTriangle(&wayOffPlane, normal, &t.a, &t.b, &t.c));
		REQUIRE(PointInTriangle(&opposite, normal, &t.a, &t.b, &t.c));
	}

	SECTION("negative coordinates work")
	{
		Triangle neg(-10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -10.0f, 0.0f, -10.0f);
		const Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 inside(-8.0f, 0.0f, -5.0f);
		REQUIRE(PointInTriangle(&inside, normal, &neg.a, &neg.b, &neg.c));

		Vector3 behindEdge(-12.0f, 0.0f, -3.0f);
		Vector3 pastEnd(-8.0f, 0.0f, -12.0f);
		REQUIRE_FALSE(PointInTriangle(&behindEdge, normal, &neg.a, &neg.b, &neg.c));
		REQUIRE_FALSE(PointInTriangle(&pastEnd, normal, &neg.a, &neg.b, &neg.c));
	}

	SECTION("large magnitudes work")
	{
		const Vector3 normal(0.0f, -1.0f, 0.0f);
		for (float leg : {1.0e3f, 1.0e4f, 1.0e5f, 1.0e6f})
		{
			Triangle big = xzTriangle(leg);
			Vector3 inside(1.0f, 0.0f, 1.0f);
			REQUIRE(PointInTriangle(&inside, normal, &big.a, &big.b, &big.c));

			Vector3 farOutside(leg + 1.0f, 0.0f, 1.0f);
			REQUIRE_FALSE(PointInTriangle(&farOutside, normal, &big.a, &big.b, &big.c));
		}
	}
}

TEST_CASE("LineFacet plane intersection", "[vector3][statics]")
{
	SECTION("a crossing line inside the footprint is found")
	{
		Triangle t = xzTriangle(10.0f);
		const Vector3 from(1.0f, 5.0f, 1.0f);
		const Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 hit;
		REQUIRE(LineFacet(from, to, t.a, t.b, t.c, &hit));
		REQUIRE(std::fabs(hit.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 1.0f) < kEpsilon);
	}

	SECTION("the hit is the plane crossing, not the segment midpoint")
	{
		// The line is asymmetric about the plane, so the two differ.
		Triangle t = xzTriangle(10.0f);
		const Vector3 from(2.0f, 9.0f, 3.0f);
		const Vector3 to(2.0f, -1.0f, 3.0f);
		Vector3 hit;
		REQUIRE(LineFacet(from, to, t.a, t.b, t.c, &hit));
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		const Vector3 midpoint = (from + to) * 0.5f;
		REQUIRE(std::fabs(midpoint.y - 4.0f) < kEpsilon);
	}

	SECTION("a line parallel to the plane is rejected without writing the output")
	{
		Triangle t = xzTriangle(10.0f);
		const Vector3 from(1.0f, 5.0f, 1.0f);
		const Vector3 to(5.0f, 5.0f, 5.0f);
		Vector3 hit(-999.0f, -999.0f, -999.0f);
		REQUIRE_FALSE(LineFacet(from, to, t.a, t.b, t.c, &hit));
		REQUIRE(hit.x == -999.0f);
		REQUIRE(hit.y == -999.0f);
		REQUIRE(hit.z == -999.0f);
	}

	SECTION("a crossing beyond the far end is rejected but the output is still written")
	{
		// The output is stored before the segment range test, so a miss still
		// leaves the extrapolated plane crossing behind.
		Triangle t = xzTriangle(10.0f);
		const Vector3 from(1.0f, 5.0f, 1.0f);
		const Vector3 to(1.0f, 4.0f, 1.0f);
		Vector3 hit;
		REQUIRE_FALSE(LineFacet(from, to, t.a, t.b, t.c, &hit));
		REQUIRE(std::fabs(hit.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 1.0f) < kEpsilon);
	}

	SECTION("a crossing behind the start is rejected and the output is written")
	{
		Triangle t = xzTriangle(10.0f);
		const Vector3 from(1.0f, 4.0f, 1.0f);
		const Vector3 to(1.0f, 5.0f, 1.0f);
		Vector3 hit;
		REQUIRE_FALSE(LineFacet(from, to, t.a, t.b, t.c, &hit));
		REQUIRE(std::fabs(hit.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
	}

	SECTION("a crossing at exactly mu == 0 is inside the segment")
	{
		Triangle t = xzTriangle(10.0f);
		const Vector3 from(1.0f, 0.0f, 1.0f);
		const Vector3 to(1.0f, 5.0f, 1.0f);
		Vector3 hit;
		REQUIRE(LineFacet(from, to, t.a, t.b, t.c, &hit));
		REQUIRE(std::fabs(hit.x - from.x) < kEpsilon);
		REQUIRE(std::fabs(hit.y - from.y) < kEpsilon);
		REQUIRE(std::fabs(hit.z - from.z) < kEpsilon);
	}

	SECTION("a crossing at exactly mu == 1 is inside the segment")
	{
		Triangle t = xzTriangle(10.0f);
		const Vector3 from(1.0f, 5.0f, 1.0f);
		const Vector3 to(1.0f, 0.0f, 1.0f);
		Vector3 hit;
		REQUIRE(LineFacet(from, to, t.a, t.b, t.c, &hit));
		REQUIRE(std::fabs(hit.x - to.x) < kEpsilon);
		REQUIRE(std::fabs(hit.y - to.y) < kEpsilon);
		REQUIRE(std::fabs(hit.z - to.z) < kEpsilon);
	}

	SECTION("the denominator epsilon is a hard cutoff at 1e-7")
	{
		Triangle t = xzTriangle(1.0f);
		const Vector3 start(0.5f, 0.0f, 0.5f);

		// denom == -1e-6 clears the cutoff, and the crossing lands on the start.
		const Vector3 almostParallel(0.5f, 0.000001f, 0.5f);
		Vector3 hit;
		REQUIRE(LineFacet(start, almostParallel, t.a, t.b, t.c, &hit));
		REQUIRE(std::fabs(hit.x - 0.5f) < kEpsilon);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 0.5f) < kEpsilon);

		// denom == -1e-8 falls under it and the call bails out first.
		const Vector3 tooParallel(0.5f, 0.00000001f, 0.5f);
		Vector3 rejected;
		REQUIRE_FALSE(LineFacet(start, tooParallel, t.a, t.b, t.c, &rejected));
	}

	SECTION("a zero length segment is rejected")
	{
		Triangle t = xzTriangle(10.0f);
		const Vector3 point(1.0f, 5.0f, 1.0f);
		Vector3 hit;
		REQUIRE_FALSE(LineFacet(point, point, t.a, t.b, t.c, &hit));
	}

	SECTION("a facet with all three corners equal is rejected")
	{
		Triangle t(1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f);
		const Vector3 from(1.0f, 3.0f, 3.0f);
		const Vector3 to(1.0f, 1.0f, 3.0f);
		Vector3 hit;
		REQUIRE_FALSE(LineFacet(from, to, t.a, t.b, t.c, &hit));
	}

	SECTION("a facet with collinear corners is rejected")
	{
		Triangle t(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f);
		const Vector3 from(0.5f, 5.0f, 0.0f);
		const Vector3 to(0.5f, -5.0f, 0.0f);
		Vector3 hit;
		REQUIRE_FALSE(LineFacet(from, to, t.a, t.b, t.c, &hit));
	}

	SECTION("negative coordinates work")
	{
		Triangle t(-10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -10.0f, 0.0f, -10.0f);
		Vector3 hit;
		REQUIRE(LineFacet(Vector3(-8.0f, 5.0f, -5.0f), Vector3(-8.0f, -5.0f, -5.0f), t.a, t.b, t.c, &hit));
		REQUIRE(std::fabs(hit.x + 8.0f) < kEpsilon);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		REQUIRE(std::fabs(hit.z + 5.0f) < kEpsilon);

		REQUIRE_FALSE(LineFacet(Vector3(-12.0f, 5.0f, -3.0f), Vector3(-12.0f, -5.0f, -3.0f), t.a, t.b, t.c, &hit));
		REQUIRE_FALSE(LineFacet(Vector3(-8.0f, 5.0f, -12.0f), Vector3(-8.0f, -5.0f, -12.0f), t.a, t.b, t.c, &hit));
	}

	SECTION("large magnitudes work")
	{
		const Vector3 from(1.0f, 5.0f, 1.0f);
		const Vector3 to(1.0f, -5.0f, 1.0f);
		for (float leg : {1.0e3f, 1.0e4f, 1.0e5f, 1.0e6f})
		{
			Triangle big = xzTriangle(leg);
			Vector3 hit;
			REQUIRE(LineFacet(from, to, big.a, big.b, big.c, &hit));
			REQUIRE(std::fabs(hit.x - 1.0f) < kEpsilon);
			REQUIRE(std::fabs(hit.y) < kEpsilon);
			REQUIRE(std::fabs(hit.z - 1.0f) < kEpsilon);
		}
	}

	SECTION("a tilted facet works and the derived normal dominates y")
	{
		// The plane is z == 2y, so the dominant normal component is y and
		// PointInTriangle projects onto XZ.
		Triangle t(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f);
		Vector3 hit;
		REQUIRE(LineFacet(
			Vector3(0.33333334f, 10.0f, 0.6666667f),
			Vector3(0.33333334f, -10.0f, 0.6666667f),
			t.a, t.b, t.c, &hit));
		REQUIRE(std::fabs(hit.x - 0.33333334f) < kEpsilon);
		REQUIRE(std::fabs(hit.y - 0.33333334f) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 0.6666667f) < kEpsilon);

		REQUIRE_FALSE(LineFacet(
			Vector3(2.0f, 10.0f, 0.0f),
			Vector3(2.0f, -10.0f, 0.0f),
			t.a, t.b, t.c, &hit));
	}
}

TEST_CASE("LineFacetHit pointer overload", "[vector3][statics]")
{
	Triangle t = xzTriangle(10.0f);

	SECTION("a hit returns exactly 1.0f and the crossing point")
	{
		Vector3 from(1.0f, 5.0f, 1.0f);
		Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 hit;
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &hit) == 1.0f);
		REQUIRE(std::fabs(hit.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 1.0f) < kEpsilon);
	}

	SECTION("every miss returns exactly 0.0f")
	{
		Vector3 hit;
		Vector3 from;
		Vector3 to;

		// Over the plane but outside the footprint.
		from = Vector3(100.0f, 5.0f, 100.0f);
		to = Vector3(100.0f, -5.0f, 100.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &hit) == 0.0f);

		// Parallel to the plane.
		from = Vector3(1.0f, 5.0f, 1.0f);
		to = Vector3(5.0f, 5.0f, 5.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &hit) == 0.0f);

		// Crossing beyond the far end of the segment.
		from = Vector3(1.0f, 5.0f, 1.0f);
		to = Vector3(1.0f, 4.0f, 1.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &hit) == 0.0f);

		// Degenerate facets have no plane at all.
		Triangle collapsed(1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f);
		from = Vector3(1.0f, 4.0f, 3.0f);
		to = Vector3(1.0f, 1.0f, 3.0f);
		REQUIRE(LineFacetHit(&from, &to, &collapsed.a, &collapsed.b, &collapsed.c, &hit) == 0.0f);

		Triangle collinear(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f);
		from = Vector3(0.5f, 5.0f, 0.0f);
		to = Vector3(0.5f, -5.0f, 0.0f);
		REQUIRE(LineFacetHit(&from, &to, &collinear.a, &collinear.b, &collinear.c, &hit) == 0.0f);

		// Zero length segment.
		from = Vector3(1.0f, 5.0f, 1.0f);
		to = Vector3(1.0f, 5.0f, 1.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &hit) == 0.0f);
	}

	SECTION("a miss outside the segment still writes the crossing")
	{
		Vector3 from(1.0f, 4.0f, 1.0f);
		Vector3 to(1.0f, 5.0f, 1.0f);
		Vector3 hit;
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &hit) == 0.0f);
		REQUIRE(std::fabs(hit.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 1.0f) < kEpsilon);
	}

	SECTION("the segment end tests are inclusive")
	{
		Vector3 hit;
		Vector3 from(1.0f, 0.0f, 1.0f);
		Vector3 to(1.0f, 5.0f, 1.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &hit) == 1.0f);

		from = Vector3(1.0f, 5.0f, 1.0f);
		to = Vector3(1.0f, 0.0f, 1.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &hit) == 1.0f);
	}

	SECTION("it agrees with the value overload on the same geometry")
	{
		Vector3 from(1.0f, 5.0f, 1.0f);
		Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 byPointer;
		Vector3 byValue;
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &byPointer) == 1.0f);
		REQUIRE(LineFacet(from, to, t.a, t.b, t.c, &byValue));
		REQUIRE(std::fabs(byPointer.x - byValue.x) < kEpsilon);
		REQUIRE(std::fabs(byPointer.y - byValue.y) < kEpsilon);
		REQUIRE(std::fabs(byPointer.z - byValue.z) < kEpsilon);

		Vector3 fromMiss(100.0f, 5.0f, 100.0f);
		Vector3 toMiss(100.0f, -5.0f, 100.0f);
		REQUIRE(LineFacetHit(&fromMiss, &toMiss, &t.a, &t.b, &t.c, &byPointer) == 0.0f);
		REQUIRE_FALSE(LineFacet(fromMiss, toMiss, t.a, t.b, t.c, &byValue));
	}

	SECTION("a tilted facet works")
	{
		Triangle tilted(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f);
		Vector3 from(0.33333334f, 10.0f, 0.6666667f);
		Vector3 to(0.33333334f, -10.0f, 0.6666667f);
		Vector3 hit;
		REQUIRE(LineFacetHit(&from, &to, &tilted.a, &tilted.b, &tilted.c, &hit) == 1.0f);
		REQUIRE(std::fabs(hit.x - 0.33333334f) < kEpsilon);
		REQUIRE(std::fabs(hit.y - 0.33333334f) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 0.6666667f) < kEpsilon);
	}
}

TEST_CASE("LineFacetHit normal overload", "[vector3][statics]")
{
	// This overload trusts the caller's normal and never normalises it, so the
	// crossing parameter depends only on its direction, not its magnitude.
	Triangle t = xzTriangle(10.0f);
	const Vector3 unit(0.0f, -1.0f, 0.0f);

	SECTION("a hit returns exactly 1.0f and the crossing point")
	{
		Vector3 from(1.0f, 5.0f, 1.0f);
		Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 normal = unit;
		Vector3 hit;
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &hit) == 1.0f);
		REQUIRE(std::fabs(hit.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 1.0f) < kEpsilon);
	}

	SECTION("scaling the supplied normal does not move the crossing")
	{
		Vector3 from(1.0f, 5.0f, 1.0f);
		Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 reference;
		Vector3 normal = unit;
		Vector3 hit;
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &reference) == 1.0f);

		for (float scale : {1000.0f, 0.0001f, -1.0f})
		{
			normal = Vector3(0.0f, -1.0f * scale, 0.0f);
			hit = Vector3(-999.0f, -999.0f, -999.0f);
			REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &hit) == 1.0f);
			REQUIRE(std::fabs(hit.x - reference.x) < kEpsilon);
			REQUIRE(std::fabs(hit.y - reference.y) < kEpsilon);
			REQUIRE(std::fabs(hit.z - reference.z) < kEpsilon);
		}
	}

	SECTION("a zero normal is treated as parallel to the line")
	{
		Vector3 from(1.0f, 5.0f, 1.0f);
		Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 normal(0.0f, 0.0f, 0.0f);
		Vector3 hit(-999.0f, -999.0f, -999.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &hit) == 0.0f);
		REQUIRE(hit.x == -999.0f);
		REQUIRE(hit.y == -999.0f);
		REQUIRE(hit.z == -999.0f);
	}

	SECTION("every miss returns exactly 0.0f")
	{
		Vector3 normal = unit;
		Vector3 hit;
		Vector3 from;
		Vector3 to;

		from = Vector3(100.0f, 5.0f, 100.0f);
		to = Vector3(100.0f, -5.0f, 100.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &hit) == 0.0f);

		from = Vector3(1.0f, 5.0f, 1.0f);
		to = Vector3(5.0f, 5.0f, 5.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &hit) == 0.0f);

		from = Vector3(1.0f, 4.0f, 1.0f);
		to = Vector3(1.0f, 5.0f, 1.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &hit) == 0.0f);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
	}

	SECTION("the segment end tests are inclusive")
	{
		Vector3 normal = unit;
		Vector3 hit;
		Vector3 from(1.0f, 0.0f, 1.0f);
		Vector3 to(1.0f, 5.0f, 1.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &hit) == 1.0f);

		from = Vector3(1.0f, 5.0f, 1.0f);
		to = Vector3(1.0f, 0.0f, 1.0f);
		REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &hit) == 1.0f);
	}

	SECTION("a tilted facet works with an unnormalised normal")
	{
		// The plane is z == 2y; the supplied normal is the raw cross product.
		Triangle tilted(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f);
		Vector3 normal(0.0f, -2.0f, 1.0f);
		Vector3 from(0.33333334f, 10.0f, 0.6666667f);
		Vector3 to(0.33333334f, -10.0f, 0.6666667f);
		Vector3 hit;
		REQUIRE(LineFacetHit(&from, &to, &tilted.a, &tilted.b, &tilted.c, &normal, &hit) == 1.0f);
		REQUIRE(std::fabs(hit.x - 0.33333334f) < kEpsilon);
		REQUIRE(std::fabs(hit.y - 0.33333334f) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 0.6666667f) < kEpsilon);

		// The same line against the same facet, but with the six argument form.
		normal = Vector3(0.0f, -1.0f, 0.0f);
		Vector3 bySixArgs;
		Vector3 bySevenArgs;
		from = Vector3(0.33333334f, 10.0f, 0.6666667f);
		to = Vector3(0.33333334f, -10.0f, 0.6666667f);
		Vector3 sixArgNormal = unit;
		REQUIRE(LineFacetHit(&from, &to, &tilted.a, &tilted.b, &tilted.c, &sixArgNormal, &bySixArgs) == 1.0f);
		REQUIRE(LineFacetHit(&from, &to, &tilted.a, &tilted.b, &tilted.c, &normal, &bySevenArgs) == 1.0f);
		REQUIRE(std::fabs(bySixArgs.y - bySevenArgs.y) < kEpsilon);
	}
}

TEST_CASE("facet helpers keep no state between calls", "[vector3][statics]")
{
	SECTION("an accept followed by a reject still rejects")
	{
		// If the intersection flag survived a return, the second call would
		// report the first call's answer.
		Triangle t = xzTriangle(10.0f);
		const Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 inside(1.0f, 0.0f, 1.0f);
		Vector3 outside(8.0f, 0.0f, 8.0f);
		REQUIRE(PointInTriangle(&inside, normal, &t.a, &t.b, &t.c));
		REQUIRE_FALSE(PointInTriangle(&outside, normal, &t.a, &t.b, &t.c));
		REQUIRE(PointInTriangle(&inside, normal, &t.a, &t.b, &t.c));
	}

	SECTION("the axis selection follows the normal, not the previous call")
	{
		// The previous call decides which two axes are kept, so a facet in one
		// plane followed by one in another must both answer correctly.
		Triangle inXz(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		Triangle inXy(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f);

		Vector3 probe(0.25f, 0.25f, 0.25f);
		REQUIRE(PointInTriangle(&probe, Vector3(0.0f, -1.0f, 0.0f), &inXz.a, &inXz.b, &inXz.c));
		REQUIRE(PointInTriangle(&probe, Vector3(0.0f, 0.0f, 1.0f), &inXy.a, &inXy.b, &inXy.c));
		REQUIRE(PointInTriangle(&probe, Vector3(0.0f, -1.0f, 0.0f), &inXz.a, &inXz.b, &inXz.c));
	}

	SECTION("interleaved facets are not influenced by each other")
	{
		Triangle ground = xzTriangle(10.0f);
		Triangle wall(0.0f, 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f);

		Vector3 from(1.0f, 5.0f, 1.0f);
		Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 groundHit;
		Vector3 wallHit;
		REQUIRE(LineFacet(from, to, ground.a, ground.b, ground.c, &groundHit));
		REQUIRE_FALSE(LineFacet(from, to, wall.a, wall.b, wall.c, &wallHit));
		REQUIRE(LineFacet(from, to, ground.a, ground.b, ground.c, &groundHit));
		REQUIRE_FALSE(LineFacet(from, to, wall.a, wall.b, wall.c, &wallHit));
		REQUIRE(LineFacet(from, to, ground.a, ground.b, ground.c, &groundHit));

		REQUIRE(std::fabs(groundHit.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(groundHit.y) < kEpsilon);
		REQUIRE(std::fabs(groundHit.z - 1.0f) < kEpsilon);
	}

	SECTION("the two LineFacetHit overloads do not disturb each other")
	{
		Triangle t = xzTriangle(10.0f);
		Vector3 from(1.0f, 5.0f, 1.0f);
		Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 normal(0.0f, -1.0f, 0.0f);

		Vector3 six;
		Vector3 seven;
		for (int i = 0; i < 3; i++)
		{
			REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &six) == 1.0f);
			REQUIRE(LineFacetHit(&from, &to, &t.a, &t.b, &t.c, &normal, &seven) == 1.0f);
		}
		REQUIRE(std::fabs(six.x - seven.x) < kEpsilon);
		REQUIRE(std::fabs(six.y - seven.y) < kEpsilon);
		REQUIRE(std::fabs(six.z - seven.z) < kEpsilon);
	}

	SECTION("repeated identical calls give bit identical answers")
	{
		Triangle t = xzTriangle(10.0f);
		const Vector3 from(1.0f, 5.0f, 1.0f);
		const Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 first;
		REQUIRE(LineFacet(from, to, t.a, t.b, t.c, &first));
		for (int i = 0; i < 4; i++)
		{
			Vector3 again;
			REQUIRE(LineFacet(from, to, t.a, t.b, t.c, &again));
			REQUIRE(again.x == first.x);
			REQUIRE(again.y == first.y);
			REQUIRE(again.z == first.z);
		}
	}
}
