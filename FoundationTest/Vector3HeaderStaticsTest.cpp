// Characterisation tests for the inline functions in Math/Vector3.hpp.
//
// Every case here passes against the current implementation, including the
// inputs that divide by zero, compare against a boundary discriminant and land
// exactly on a tangent. They exist so that deleting the function-local statics
// can be shown to change nothing observable.

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>

#include "Math/Vector3.hpp"

namespace
{

constexpr float kEpsilon = 1e-4f;
constexpr float kPi = std::numbers::pi_v<float>;

// ReflectVector reflects about a plane with normal n, so for a unit normal the
// result is v - 2 * (n.v) * n. This is the reference the tests compare against.
void reflect(Vector3* velocity, const Vector3& unitNormal)
{
	const float d = unitNormal.x * velocity->x + unitNormal.y * velocity->y + unitNormal.z * velocity->z;
	velocity->x -= 2.0f * d * unitNormal.x;
	velocity->y -= 2.0f * d * unitNormal.y;
	velocity->z -= 2.0f * d * unitNormal.z;
}

void checkSphereLineCase(
	const Vector3& p1, const Vector3& p2, const Vector3& p3,
	float r, bool expected)
{
	Vector3 a = p1;
	Vector3 b = p2;
	Vector3 centre = p3;
	float radius = r;

	const bool pointerForm = sphere_line_intersection(&a, &b, &centre, &radius);
	const bool scalarForm = sphere_line_intersection(
		p1.x, p1.y, p1.z,
		p2.x, p2.y, p2.z,
		p3.x, p3.y, p3.z,
		r);

	REQUIRE(pointerForm == expected);
	REQUIRE(scalarForm == expected);
}

} // namespace

TEST_CASE("Normalise", "[vector3][headerstatics]")
{
	SECTION("an axis aligned vector becomes a unit vector")
	{
		Vector3 v(5.0f, 0.0f, 0.0f);
		Normalise(&v);
		REQUIRE(v.x == 1.0f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("the length is removed, not just scaled down")
	{
		Vector3 v(3.0f, 0.0f, 4.0f);
		Normalise(&v);
		REQUIRE(std::fabs(findLength(&v) - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(v.x - 0.6f) < kEpsilon);
		REQUIRE(std::fabs(v.z - 0.8f) < kEpsilon);
	}

	SECTION("negative coordinates normalise")
	{
		Vector3 v(-0.0f, -5.0f, 0.0f);
		Normalise(&v);
		REQUIRE(std::fabs(v.y + 1.0f) < kEpsilon);
	}

	SECTION("the zero vector is left untouched instead of producing NaN")
	{
		Vector3 v(0.0f, 0.0f, 0.0f);
		Normalise(&v);
		REQUIRE(v.x == 0.0f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("an already normalised vector keeps its direction under a second pass")
	{
		// Not bit identical: the first pass leaves a length that is one ulp off
		// 1.0f, so the second pass rescales by that ulp.
		Vector3 v(0.0f, 0.57735026f, 0.57735026f);
		Normalise(&v);
		const Vector3 once = v;
		Normalise(&v);
		REQUIRE(std::fabs(v.x - once.x) < kEpsilon);
		REQUIRE(std::fabs(v.y - once.y) < kEpsilon);
		REQUIRE(std::fabs(v.z - once.z) < kEpsilon);
		REQUIRE(std::fabs(findLength(&v) - 1.0f) < kEpsilon);
	}

	SECTION("a large magnitude normalises to the same direction")
	{
		Vector3 big(3.0e18f, 0.0f, 4.0e18f);
		Normalise(&big);
		REQUIRE(std::fabs(big.x - 0.6f) < kEpsilon);
		REQUIRE(std::fabs(big.z - 0.8f) < kEpsilon);
	}

	SECTION("a magnitude whose square underflows to zero is left alone")
	{
		// The guard is an exact d == 0 test, and (1e-30)^2 is subnormal enough to
		// flush to zero in a float, so a very small vector is *not* normalised.
		Vector3 v(1.0e-30f, 0.0f, 0.0f);
		Normalise(&v);
		REQUIRE(v.x == 1.0e-30f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("the smallest magnitude with a normal square still normalises")
	{
		Vector3 v(1.0e-15f, 0.0f, 0.0f);
		Normalise(&v);
		REQUIRE(v.x == 1.0f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("the length is recomputed per call, not carried over")
	{
		Vector3 first(10.0f, 0.0f, 0.0f);
		Normalise(&first);
		Vector3 second(0.0f, 0.0f, 7.0f);
		Normalise(&second);

		REQUIRE(std::fabs(first.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(second.z - 1.0f) < kEpsilon);
	}
}

TEST_CASE("Vector3 compound add and subtract", "[vector3][headerstatics]")
{
	SECTION("+= is componentwise and in place")
	{
		Vector3 v(1.0f, -2.0f, 3.5f);
		v += Vector3(0.5f, 2.0f, -3.5f);
		REQUIRE(v.x == 1.5f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("-= is componentwise and in place")
	{
		Vector3 v(1.0f, -2.0f, 3.5f);
		v -= Vector3(0.5f, 2.0f, -3.5f);
		REQUIRE(v.x == 0.5f);
		REQUIRE(v.y == -4.0f);
		REQUIRE(v.z == 7.0f);
	}

	SECTION("+= against the zero vector is a no-op")
	{
		Vector3 v(-1.0f, 0.0f, 2.0f);
		v += Vector3();
		REQUIRE(v.x == -1.0f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 2.0f);
	}

	SECTION("-= against the zero vector is a no-op")
	{
		Vector3 v(-1.0f, 0.0f, 2.0f);
		v -= Vector3();
		REQUIRE(v.x == -1.0f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 2.0f);
	}

	SECTION("self compound assignment zeroes or doubles the vector")
	{
		Vector3 v(1.0f, -2.0f, 4.0f);
		v += v;
		REQUIRE(v.x == 2.0f);
		REQUIRE(v.y == -4.0f);
		REQUIRE(v.z == 8.0f);

		v -= v;
		REQUIRE(v.x == 0.0f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("large magnitudes accumulate without being clamped")
	{
		Vector3 v(1.0e18f, -1.0e18f, 1.0e18f);
		v += Vector3(1.0e18f, 1.0e18f, -1.0e18f);
		REQUIRE(v.x == 2.0e18f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("a sequence of operations lands on the algebraically exact value")
	{
		Vector3 v(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < 5; i++) {
			v += Vector3(1.0f, 2.0f, 3.0f);
		}
		for (int i = 0; i < 3; i++) {
			v -= Vector3(1.0f, 2.0f, 3.0f);
		}
		REQUIRE(v.x == 2.0f);
		REQUIRE(v.y == 4.0f);
		REQUIRE(v.z == 6.0f);
	}
}

TEST_CASE("Vector3 operator/ and dot products", "[vector3][headerstatics]")
{
	SECTION("division by a scalar divides every component")
	{
		const Vector3 v(1.0f, -2.0f, 4.0f);
		const Vector3 half = v / 2.0f;
		REQUIRE(half.x == 0.5f);
		REQUIRE(half.y == -1.0f);
		REQUIRE(half.z == 2.0f);
	}

	SECTION("division by a negative scalar flips the sign of every component")
	{
		const Vector3 v(1.0f, -2.0f, 4.0f);
		const Vector3 flipped = v / -0.5f;
		REQUIRE(flipped.x == -2.0f);
		REQUIRE(flipped.y == 4.0f);
		REQUIRE(flipped.z == -8.0f);
	}

	SECTION("division by zero yields an infinity rather than being guarded")
	{
		// Read the divisor through a volatile so the optimiser cannot fold it to a
		// literal zero and reject the division as undefined; the float handed to
		// operator/ is still exactly 0.0f.
		volatile float zero = 0.0f;
		const Vector3 v(1.0f, 0.0f, -4.0f);
		const Vector3 divided = v / zero;
		REQUIRE(std::isinf(divided.x));
		REQUIRE(std::isnan(divided.y));
		REQUIRE(std::isinf(divided.z));
	}

	SECTION("consecutive divisions do not contaminate each other")
	{
		const Vector3 v(8.0f, -8.0f, 4.0f);
		const Vector3 first = v / 4.0f;
		const Vector3 second = v / 2.0f;
		const Vector3 third = v / 8.0f;

		REQUIRE(first.x == 2.0f);
		REQUIRE(first.y == -2.0f);
		REQUIRE(first.z == 1.0f);
		REQUIRE(second.x == 4.0f);
		REQUIRE(third.z == 0.5f);
	}

	SECTION("division matches repeated scaling")
	{
		const Vector3 v(1.0f, 3.0f, -7.0f);
		const Vector3 scaled = v * 0.125f;
		const Vector3 divided = v / 8.0f;
		REQUIRE(divided.x == scaled.x);
		REQUIRE(divided.y == scaled.y);
		REQUIRE(divided.z == scaled.z);
	}

	SECTION("dotproduct is the plain dot product")
	{
		const Vector3 a(1.0f, 2.0f, 3.0f);
		const Vector3 b(4.0f, -5.0f, 6.0f);
		REQUIRE(dotproduct(&a, &b) == 12.0f);
		REQUIRE(dotproduct(&b, &a) == 12.0f);
	}

	SECTION("dotproduct with the zero vector is zero")
	{
		const Vector3 a(1.0f, 2.0f, 3.0f);
		const Vector3 zero;
		REQUIRE(dotproduct(&a, &zero) == 0.0f);
		REQUIRE(dotproduct(&zero, &a) == 0.0f);
		REQUIRE(dotproduct(&zero, &zero) == 0.0f);
	}

	SECTION("dotproduct against itself is the squared magnitude")
	{
		const Vector3 a(1.0f, -2.0f, 3.0f);
		REQUIRE(std::fabs(dotproduct(&a, &a) - 14.0f) < kEpsilon);
	}

	SECTION("interleaved dotproducts do not reuse the previous answer")
	{
		const Vector3 a(1.0f, 0.0f, 0.0f);
		const Vector3 b(0.0f, 1.0f, 0.0f);
		const Vector3 c(0.0f, 0.0f, 1.0f);
		REQUIRE(dotproduct(&a, &b) == 0.0f);
		REQUIRE(dotproduct(&b, &c) == 0.0f);
		REQUIRE(dotproduct(&a, &a) == 1.0f);
		REQUIRE(dotproduct(&b, &b) == 1.0f);
		REQUIRE(dotproduct(&a, &c) == 0.0f);
	}

	SECTION("normaldotproduct is the cosine of the angle between two vectors")
	{
		const Vector3 a(3.0f, 0.0f, 4.0f);
		const Vector3 b(0.0f, 0.0f, 10.0f);
		REQUIRE(std::fabs(normaldotproduct(a, b) - 0.8f) < kEpsilon);

		const Vector3 c(-3.0f, 0.0f, -4.0f);
		REQUIRE(std::fabs(normaldotproduct(a, c) + 1.0f) < kEpsilon);
	}

	SECTION("normaldotproduct ignores magnitude")
	{
		const Vector3 a(1.0f, 2.0f, 3.0f);
		const Vector3 b(1.0f, 2.0f, 3.0f);
		const Vector3 scaled(1000.0f, 2000.0f, 3000.0f);
		REQUIRE(std::fabs(normaldotproduct(a, b) - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(normaldotproduct(a, scaled) - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(normaldotproduct(scaled, a) - 1.0f) < kEpsilon);
	}

	SECTION("normaldotproduct leaves its arguments alone")
	{
		// Both parameters are taken by value and normalised through their local
		// copies, so the caller's vectors must survive untouched.
		Vector3 a(3.0f, 0.0f, 4.0f);
		Vector3 b(0.0f, 0.0f, 10.0f);
		normaldotproduct(a, b);
		REQUIRE(a.x == 3.0f);
		REQUIRE(a.y == 0.0f);
		REQUIRE(a.z == 4.0f);
		REQUIRE(b.z == 10.0f);
	}

	SECTION("normaldotproduct treats the zero vector as contributing nothing")
	{
		const Vector3 a(1.0f, 2.0f, 3.0f);
		const Vector3 zero;
		// Normalise leaves the zero vector at zero rather than producing NaN, so
		// the answer is a clean zero rather than a NaN.
		REQUIRE(normaldotproduct(a, zero) == 0.0f);
		REQUIRE(normaldotproduct(zero, a) == 0.0f);
		REQUIRE(normaldotproduct(zero, zero) == 0.0f);
	}

	SECTION("interleaved normaldotproducts do not reuse the previous answer")
	{
		const Vector3 x(1.0f, 0.0f, 0.0f);
		const Vector3 y(0.0f, 1.0f, 0.0f);
		const Vector3 z(0.0f, 0.0f, 1.0f);
		REQUIRE(std::fabs(normaldotproduct(x, x) - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(normaldotproduct(x, y)) < kEpsilon);
		REQUIRE(std::fabs(normaldotproduct(y, z)) < kEpsilon);
		REQUIRE(std::fabs(normaldotproduct(z, x)) < kEpsilon);
		REQUIRE(std::fabs(normaldotproduct(y, y) - 1.0f) < kEpsilon);
	}
}

TEST_CASE("ReflectVector", "[vector3][headerstatics]")
{
	SECTION("normal incidence reverses only the normal component")
	{
		Vector3 v(0.0f, -1.0f, 0.0f);
		const Vector3 n(0.0f, 1.0f, 0.0f);
		ReflectVector(&v, n);
		REQUIRE(v.x == 0.0f);
		REQUIRE(std::fabs(v.y - 1.0f) < kEpsilon);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("grazing incidence leaves the vector alone")
	{
		Vector3 v(1.0f, 0.0f, 0.0f);
		const Vector3 n(0.0f, 1.0f, 0.0f);
		ReflectVector(&v, n);
		REQUIRE(std::fabs(v.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(v.y) < kEpsilon);
		REQUIRE(std::fabs(v.z) < kEpsilon);
	}

	SECTION("45 degree incidence flips the normal component and keeps the rest")
	{
		Vector3 v(1.0f, -1.0f, 0.0f);
		const Vector3 n(0.0f, 1.0f, 0.0f);
		ReflectVector(&v, n);
		REQUIRE(std::fabs(v.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(v.y - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(v.z) < kEpsilon);
	}

	SECTION("an oblique incidence on a tilted plane matches the reference formula")
	{
		const Vector3 n = Vector3(1.0f, 1.0f, 0.0f);
		Vector3 unitNormal = n;
		Normalise(&unitNormal);

		Vector3 v(0.6f, -0.8f, 0.0f);
		Vector3 expected = v;
		reflect(&expected, unitNormal);
		ReflectVector(&v, unitNormal);

		REQUIRE(std::fabs(v.x - expected.x) < kEpsilon);
		REQUIRE(std::fabs(v.y - expected.y) < kEpsilon);
		REQUIRE(std::fabs(v.z - expected.z) < kEpsilon);
	}

	SECTION("a tangential velocity is unchanged on a tilted plane")
	{
		const Vector3 n(1.0f, 1.0f, 0.0f);
		Vector3 unitNormal = n;
		Normalise(&unitNormal);

		Vector3 v(1.0f, -1.0f, 0.0f);
		const Vector3 before = v;
		ReflectVector(&v, unitNormal);

		REQUIRE(std::fabs(v.x - before.x) < kEpsilon);
		REQUIRE(std::fabs(v.y - before.y) < kEpsilon);
		REQUIRE(std::fabs(v.z - before.z) < kEpsilon);
	}

	SECTION("reflection preserves the speed")
	{
		const Vector3 n(0.0f, 1.0f, 0.0f);
		for (float speed : {0.001f, 1.0f, 25.0f, 1.0e5f})
		{
			Vector3 v(speed * 0.6f, -speed * 0.8f, 0.0f);
			const float before = findLength(&v);
			ReflectVector(&v, n);
			REQUIRE(std::fabs(findLength(&v) - before) < kEpsilon * before);
		}
	}

	SECTION("a zero length velocity stays zero")
	{
		Vector3 v(0.0f, 0.0f, 0.0f);
		const Vector3 n(0.3f, -0.9f, 0.3f);
		ReflectVector(&v, n);
		REQUIRE(v.x == 0.0f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("a zero normal makes the call a no-op")
	{
		Vector3 v(1.0f, -2.0f, 3.0f);
		const Vector3 n(0.0f, 0.0f, 0.0f);
		ReflectVector(&v, n);
		REQUIRE(v.x == 1.0f);
		REQUIRE(v.y == -2.0f);
		REQUIRE(v.z == 3.0f);
	}

	SECTION("negative coordinates reflect correctly")
	{
		Vector3 v(0.0f, 0.0f, -5.0f);
		const Vector3 n(0.0f, 0.0f, 1.0f);
		ReflectVector(&v, n);
		REQUIRE(std::fabs(v.x) < kEpsilon);
		REQUIRE(std::fabs(v.y) < kEpsilon);
		REQUIRE(std::fabs(v.z - 5.0f) < kEpsilon);
	}

	SECTION("flipping the normal gives the same reflection")
	{
		Vector3 up(0.0f, -1.0f, 0.0f);
		Vector3 down(0.0f, -1.0f, 0.0f);
		ReflectVector(&up, Vector3(0.0f, 1.0f, 0.0f));
		ReflectVector(&down, Vector3(0.0f, -1.0f, 0.0f));

		REQUIRE(std::fabs(up.x - down.x) < kEpsilon);
		REQUIRE(std::fabs(up.y - down.y) < kEpsilon);
		REQUIRE(std::fabs(up.z - down.z) < kEpsilon);
	}

	SECTION("a non unit normal scales the result by its excess magnitude")
	{
		// The function never normalises n, so v - 2 * (n.v) * n grows with |n|.
		// Callers are expected to pass a unit normal; this pins the behaviour so
		// that quietly normalising inside would be a visible change.
		Vector3 v(1.0f, -1.0f, 0.0f);
		const Vector3 n(0.0f, 2.0f, 0.0f);
		ReflectVector(&v, n);
		REQUIRE(std::fabs(v.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(v.y - 7.0f) < kEpsilon);
		REQUIRE(std::fabs(v.z) < kEpsilon);
	}

	SECTION("large magnitudes reflect correctly")
	{
		Vector3 v(1.0e6f, -1.0e6f, 0.0f);
		const Vector3 n(0.0f, 1.0f, 0.0f);
		ReflectVector(&v, n);
		REQUIRE(std::fabs(v.x - 1.0e6f) < 1.0f);
		REQUIRE(std::fabs(v.y - 1.0e6f) < 1.0f);
		REQUIRE(std::fabs(v.z) < 1.0f);
	}

	SECTION("reflecting twice returns the original velocity")
	{
		const Vector3 n(0.0f, 1.0f, 0.0f);
		Vector3 v(1.0f, -1.0f, 0.5f);
		const Vector3 before = v;
		ReflectVector(&v, n);
		ReflectVector(&v, n);

		REQUIRE(std::fabs(v.x - before.x) < kEpsilon);
		REQUIRE(std::fabs(v.y - before.y) < kEpsilon);
		REQUIRE(std::fabs(v.z - before.z) < kEpsilon);
	}

	SECTION("the pointer overload agrees with the reference overload")
	{
		Vector3 byPointer(0.6f, -0.8f, 0.3f);
		Vector3 byReference(0.6f, -0.8f, 0.3f);
		Vector3 n(1.0f, 1.0f, 0.0f);

		ReflectVector(&byPointer, &n);
		ReflectVector(&byReference, n);

		REQUIRE(std::fabs(byPointer.x - byReference.x) < kEpsilon);
		REQUIRE(std::fabs(byPointer.y - byReference.y) < kEpsilon);
		REQUIRE(std::fabs(byPointer.z - byReference.z) < kEpsilon);
	}

	SECTION("interleaved normals do not leak into each other")
	{
		// A stale scratch vector from the previous plane would corrupt this one.
		Vector3 onFloor(0.0f, -1.0f, 0.0f);
		Vector3 onWall(1.0f, 0.0f, 0.0f);
		Vector3 onFloorAgain(0.0f, -1.0f, 0.0f);

		ReflectVector(&onFloor, Vector3(0.0f, 1.0f, 0.0f));
		ReflectVector(&onWall, Vector3(1.0f, 0.0f, 0.0f));
		ReflectVector(&onFloorAgain, Vector3(0.0f, 1.0f, 0.0f));

		REQUIRE(std::fabs(onFloor.y - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(onWall.x + 1.0f) < kEpsilon);
		REQUIRE(std::fabs(onFloorAgain.y - 1.0f) < kEpsilon);
	}

	SECTION("repeated identical calls give bit identical answers")
	{
		const Vector3 n(0.0f, 1.0f, 0.0f);
		Vector3 first(1.0f, -1.0f, 0.5f);
		ReflectVector(&first, n);
		for (int i = 0; i < 4; i++)
		{
			Vector3 again(1.0f, -1.0f, 0.5f);
			ReflectVector(&again, n);
			REQUIRE(again.x == first.x);
			REQUIRE(again.y == first.y);
			REQUIRE(again.z == first.z);
		}
	}
}

TEST_CASE("DoRotation", "[vector3][headerstatics]")
{
	SECTION("a zero rotation is the identity")
	{
		const Vector3 v(1.0f, -2.0f, 3.0f);
		const Vector3 rotated = DoRotation(v, 0.0f, 0.0f, 0.0f);
		REQUIRE(rotated.x == v.x);
		REQUIRE(rotated.y == v.y);
		REQUIRE(rotated.z == v.z);
	}

	SECTION("a quarter turn about Y maps +X onto -Z")
	{
		const Vector3 rotated = DoRotation(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 90.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y) < kEpsilon);
		REQUIRE(std::fabs(rotated.z + 1.0f) < kEpsilon);
	}

	SECTION("a half turn about Y maps +X onto -X")
	{
		const Vector3 rotated = DoRotation(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 180.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x + 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.y) < kEpsilon);
		REQUIRE(std::fabs(rotated.z) < kEpsilon);
	}

	SECTION("three quarter turns about Y map +X onto +Z")
	{
		const Vector3 rotated = DoRotation(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 270.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y) < kEpsilon);
		REQUIRE(std::fabs(rotated.z - 1.0f) < kEpsilon);
	}

	SECTION("a full turn about Y is the identity")
	{
		// The degrees to radians constant is the truncated 6.283185f, so a full
		// turn lands within about a micro radian rather than exactly on 2*pi.
		const Vector3 rotated = DoRotation(Vector3(1.0f, 2.0f, 3.0f), 0.0f, 360.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.y - 2.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z - 3.0f) < kEpsilon);
	}

	SECTION("a negative quarter turn about Y is the inverse")
	{
		const Vector3 forward = DoRotation(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 90.0f, 0.0f);
		const Vector3 backward = DoRotation(Vector3(1.0f, 0.0f, 0.0f), 0.0f, -90.0f, 0.0f);

		REQUIRE(std::fabs(forward.x) < kEpsilon);
		REQUIRE(std::fabs(forward.z + 1.0f) < kEpsilon);
		REQUIRE(std::fabs(backward.x) < kEpsilon);
		REQUIRE(std::fabs(backward.z - 1.0f) < kEpsilon);
	}

	SECTION("a quarter turn about X maps +Z onto -Y")
	{
		const Vector3 rotated = DoRotation(Vector3(0.0f, 0.0f, 1.0f), 90.0f, 0.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y + 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z) < kEpsilon);
	}

	SECTION("a quarter turn about Z maps +X onto +Y")
	{
		const Vector3 rotated = DoRotation(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 0.0f, 90.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z) < kEpsilon);
	}

	SECTION("the Y rotation carries the x and z components together")
	{
		const Vector3 rotated = DoRotation(Vector3(1.0f, 1.0f, 0.0f), 0.0f, 90.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z + 1.0f) < kEpsilon);
	}

	SECTION("the zero vector stays zero at every angle")
	{
		for (float angle : {0.0f, 45.0f, 90.0f, 180.0f, 270.0f, 360.0f})
		{
			const Vector3 rotated = DoRotation(Vector3(), angle, angle, angle);
			REQUIRE(rotated.x == 0.0f);
			REQUIRE(rotated.y == 0.0f);
			REQUIRE(rotated.z == 0.0f);
		}
	}

	SECTION("rotation preserves length on every axis")
	{
		const Vector3 v(1.0f, -2.0f, 3.0f);
		const float before = findLength(&v);
		for (float angle : {0.0f, 1.0f, 45.0f, 90.0f, 137.5f, 180.0f, 270.0f, 359.0f})
		{
			const Vector3 rotated = DoRotation(v, angle, angle, angle);
			REQUIRE(std::fabs(findLength(&rotated) - before) < kEpsilon * before);
		}
	}

	SECTION("large magnitudes survive a rotation")
	{
		const Vector3 v(1.0e6f, 2.0e6f, -3.0e6f);
		const float before = findLength(&v);
		const Vector3 rotated = DoRotation(v, 30.0f, 45.0f, 60.0f);
		REQUIRE(std::fabs(findLength(&rotated) - before) < 1.0f);
	}

	SECTION("negative coordinates rotate")
	{
		// The +90 degree turn about Y maps +X to -Z, so -X maps to +Z.
		const Vector3 rotated = DoRotation(Vector3(-1.0f, 0.0f, 0.0f), 0.0f, 90.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y) < kEpsilon);
		REQUIRE(std::fabs(rotated.z - 1.0f) < kEpsilon);
	}

	SECTION("the argument vector is not modified")
	{
		Vector3 v(1.0f, 2.0f, 3.0f);
		DoRotation(v, 90.0f, 90.0f, 90.0f);
		REQUIRE(v.x == 1.0f);
		REQUIRE(v.y == 2.0f);
		REQUIRE(v.z == 3.0f);
	}

	SECTION("all three axes combined agree with the radian variant")
	{
		const Vector3 degrees = DoRotation(Vector3(1.0f, 2.0f, 3.0f), 90.0f, 90.0f, 90.0f);
		const Vector3 radians = DoRotationRadian(Vector3(1.0f, 2.0f, 3.0f), kPi / 2.0f, kPi / 2.0f, kPi / 2.0f);
		REQUIRE(std::fabs(degrees.x - radians.x) < kEpsilon);
		REQUIRE(std::fabs(degrees.y - radians.y) < kEpsilon);
		REQUIRE(std::fabs(degrees.z - radians.z) < kEpsilon);
	}

	SECTION("interleaved rotations do not contaminate each other")
	{
		const Vector3 unit(1.0f, 0.0f, 0.0f);
		const Vector3 identity = DoRotation(unit, 0.0f, 0.0f, 0.0f);
		const Vector3 quarter = DoRotation(unit, 0.0f, 90.0f, 0.0f);
		const Vector3 identityAgain = DoRotation(unit, 0.0f, 0.0f, 0.0f);

		REQUIRE(identity.x == 1.0f);
		REQUIRE(identity.z == 0.0f);
		REQUIRE(std::fabs(quarter.z + 1.0f) < kEpsilon);
		REQUIRE(identityAgain.x == identity.x);
		REQUIRE(identityAgain.y == identity.y);
		REQUIRE(identityAgain.z == identity.z);
	}

	SECTION("repeated identical calls give bit identical answers")
	{
		const Vector3 v(1.0f, 2.0f, 3.0f);
		const Vector3 first = DoRotation(v, 17.0f, 43.0f, 91.0f);
		for (int i = 0; i < 4; i++)
		{
			const Vector3 again = DoRotation(v, 17.0f, 43.0f, 91.0f);
			REQUIRE(again.x == first.x);
			REQUIRE(again.y == first.y);
			REQUIRE(again.z == first.z);
		}
	}
}

TEST_CASE("DoRotationRadian", "[vector3][headerstatics]")
{
	SECTION("a zero rotation is the identity")
	{
		const Vector3 v(1.0f, -2.0f, 3.0f);
		const Vector3 rotated = DoRotationRadian(v, 0.0f, 0.0f, 0.0f);
		REQUIRE(rotated.x == v.x);
		REQUIRE(rotated.y == v.y);
		REQUIRE(rotated.z == v.z);
	}

	SECTION("a quarter turn about Y maps +X onto -Z")
	{
		const Vector3 rotated = DoRotationRadian(Vector3(1.0f, 0.0f, 0.0f), 0.0f, kPi / 2.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y) < kEpsilon);
		REQUIRE(std::fabs(rotated.z + 1.0f) < kEpsilon);
	}

	SECTION("a half turn about Y maps +X onto -X")
	{
		const Vector3 rotated = DoRotationRadian(Vector3(1.0f, 0.0f, 0.0f), 0.0f, kPi, 0.0f);
		REQUIRE(std::fabs(rotated.x + 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.y) < kEpsilon);
		REQUIRE(std::fabs(rotated.z) < kEpsilon);
	}

	SECTION("three quarter turns about Y map +X onto +Z")
	{
		const Vector3 rotated = DoRotationRadian(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 3.0f * kPi / 2.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y) < kEpsilon);
		REQUIRE(std::fabs(rotated.z - 1.0f) < kEpsilon);
	}

	SECTION("a full turn about Y is the identity")
	{
		const Vector3 rotated = DoRotationRadian(Vector3(1.0f, 2.0f, 3.0f), 0.0f, 2.0f * kPi, 0.0f);
		REQUIRE(std::fabs(rotated.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.y - 2.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z - 3.0f) < kEpsilon);
	}

	SECTION("a negative quarter turn about Y is the inverse")
	{
		const Vector3 forward = DoRotationRadian(Vector3(1.0f, 0.0f, 0.0f), 0.0f, kPi / 2.0f, 0.0f);
		const Vector3 backward = DoRotationRadian(Vector3(1.0f, 0.0f, 0.0f), 0.0f, -kPi / 2.0f, 0.0f);

		REQUIRE(std::fabs(forward.x) < kEpsilon);
		REQUIRE(std::fabs(forward.z + 1.0f) < kEpsilon);
		REQUIRE(std::fabs(backward.x) < kEpsilon);
		REQUIRE(std::fabs(backward.z - 1.0f) < kEpsilon);
	}

	SECTION("a quarter turn about X maps +Z onto -Y")
	{
		const Vector3 rotated = DoRotationRadian(Vector3(0.0f, 0.0f, 1.0f), kPi / 2.0f, 0.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y + 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z) < kEpsilon);
	}

	SECTION("a quarter turn about Z maps +X onto +Y")
	{
		const Vector3 rotated = DoRotationRadian(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 0.0f, kPi / 2.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z) < kEpsilon);
	}

	SECTION("a negative quarter turn about Z goes the other way")
	{
		const Vector3 rotated = DoRotationRadian(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 0.0f, -kPi / 2.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y + 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z) < kEpsilon);
	}

	SECTION("the Y rotation carries the x and z components together")
	{
		const Vector3 rotated = DoRotationRadian(Vector3(1.0f, 1.0f, 0.0f), 0.0f, kPi / 2.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z + 1.0f) < kEpsilon);
	}

	SECTION("the zero vector stays zero at every angle")
	{
		for (float angle : {0.0f, 0.1f, kPi / 4.0f, kPi / 2.0f, kPi, 3.0f * kPi / 2.0f, 2.0f * kPi})
		{
			const Vector3 rotated = DoRotationRadian(Vector3(), angle, angle, angle);
			REQUIRE(rotated.x == 0.0f);
			REQUIRE(rotated.y == 0.0f);
			REQUIRE(rotated.z == 0.0f);
		}
	}

	SECTION("rotation preserves length on every axis")
	{
		const Vector3 v(1.0f, -2.0f, 3.0f);
		const float before = findLength(&v);
		for (float angle : {0.0f, 0.05f, 0.785f, 1.57f, 2.4f, 3.14f, 4.71f, 6.27f})
		{
			const Vector3 rotated = DoRotationRadian(v, angle, angle, angle);
			REQUIRE(std::fabs(findLength(&rotated) - before) < kEpsilon * before);
		}
	}

	SECTION("large magnitudes survive a rotation")
	{
		const Vector3 v(1.0e6f, 2.0e6f, -3.0e6f);
		const float before = findLength(&v);
		const Vector3 rotated = DoRotationRadian(v, 0.5235988f, 0.7853982f, 1.0471976f);
		REQUIRE(std::fabs(findLength(&rotated) - before) < 1.0f);
	}

	SECTION("negative coordinates rotate")
	{
		// The +90 degree turn about Y maps +X to -Z, so -X maps to +Z.
		const Vector3 rotated = DoRotationRadian(Vector3(-1.0f, 0.0f, 0.0f), 0.0f, kPi / 2.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y) < kEpsilon);
		REQUIRE(std::fabs(rotated.z - 1.0f) < kEpsilon);
	}

	SECTION("the argument vector is not modified")
	{
		Vector3 v(1.0f, 2.0f, 3.0f);
		DoRotationRadian(v, kPi / 2.0f, kPi / 2.0f, kPi / 2.0f);
		REQUIRE(v.x == 1.0f);
		REQUIRE(v.y == 2.0f);
		REQUIRE(v.z == 3.0f);
	}

	SECTION("the degrees variant agrees at a quarter turn on every axis")
	{
		const Vector3 v(1.0f, -2.0f, 3.0f);
		const Vector3 degrees = DoRotation(v, 90.0f, 90.0f, 90.0f);
		const Vector3 radians = DoRotationRadian(v, kPi / 2.0f, kPi / 2.0f, kPi / 2.0f);
		REQUIRE(std::fabs(degrees.x - radians.x) < kEpsilon);
		REQUIRE(std::fabs(degrees.y - radians.y) < kEpsilon);
		REQUIRE(std::fabs(degrees.z - radians.z) < kEpsilon);
	}

	SECTION("interleaved rotations do not contaminate each other")
	{
		const Vector3 unit(1.0f, 0.0f, 0.0f);
		const Vector3 identity = DoRotationRadian(unit, 0.0f, 0.0f, 0.0f);
		const Vector3 quarter = DoRotationRadian(unit, 0.0f, kPi / 2.0f, 0.0f);
		const Vector3 identityAgain = DoRotationRadian(unit, 0.0f, 0.0f, 0.0f);

		REQUIRE(identity.x == 1.0f);
		REQUIRE(identity.z == 0.0f);
		REQUIRE(std::fabs(quarter.z + 1.0f) < kEpsilon);
		REQUIRE(identityAgain.x == identity.x);
		REQUIRE(identityAgain.y == identity.y);
		REQUIRE(identityAgain.z == identity.z);
	}

	SECTION("repeated identical calls give bit identical answers")
	{
		const Vector3 v(1.0f, 2.0f, 3.0f);
		const Vector3 first = DoRotationRadian(v, 0.3f, 0.75f, 1.6f);
		for (int i = 0; i < 4; i++)
		{
			const Vector3 again = DoRotationRadian(v, 0.3f, 0.75f, 1.6f);
			REQUIRE(again.x == first.x);
			REQUIRE(again.y == first.y);
			REQUIRE(again.z == first.z);
		}
	}
}

TEST_CASE("sphere_line_intersection", "[vector3][headerstatics]")
{
	SECTION("a line straight through the centre of the sphere hits")
	{
		checkSphereLineCase(
			Vector3(-5.0f, 0.0f, 0.0f), Vector3(5.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);
	}

	SECTION("a line at exactly the radius is tangent and counts as a hit")
	{
		// The discriminant is exactly zero here and the test is a strict i < 0,
		// so a tangent is a hit rather than a miss.
		checkSphereLineCase(
			Vector3(-5.0f, 1.0f, 0.0f), Vector3(5.0f, 1.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);
	}

	SECTION("a hair inside the tangent still hits")
	{
		checkSphereLineCase(
			Vector3(-5.0f, 0.9999f, 0.0f), Vector3(5.0f, 0.9999f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);
	}

	SECTION("a hair outside the tangent misses")
	{
		checkSphereLineCase(
			Vector3(-5.0f, 1.0001f, 0.0f), Vector3(5.0f, 1.0001f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("a line well clear of the sphere misses")
	{
		checkSphereLineCase(
			Vector3(-5.0f, 3.0f, 0.0f), Vector3(5.0f, 3.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("an offset centre is handled")
	{
		checkSphereLineCase(
			Vector3(4.0f, 5.0f, 5.0f), Vector3(6.0f, 5.0f, 5.0f),
			Vector3(5.0f, 5.0f, 5.0f), 2.0f, true);

		checkSphereLineCase(
			Vector3(3.0f, 5.0f, 5.0f), Vector3(7.0f, 5.0f, 5.0f),
			Vector3(5.0f, 5.0f, 5.0f), 1.0f, true);
	}

	SECTION("a line that clears the box test but misses the sphere is rejected")
	{
		// Both endpoints sit exactly on the +1 box boundary, so the strict box
		// comparisons let it through, and the discriminant is then negative.
		checkSphereLineCase(
			Vector3(1.0f, 1.0f, 1.0f), Vector3(1.0f, 1.0f, -1.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("the infinite line is tested, not the segment")
	{
		// The line y = 0.5x + 0.6 passes within 0.54 of the origin, so it hits,
		// but the closest point on this segment is the start at distance 1.08,
		// so the segment itself misses. The function answers true.
		checkSphereLineCase(
			Vector3(0.6f, 0.9f, 0.0f), Vector3(4.0f, 2.6f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);
	}

	SECTION("both endpoints beyond the sphere on +X are rejected by the box test")
	{
		checkSphereLineCase(
			Vector3(10.0f, 0.0f, 0.0f), Vector3(11.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("both endpoints beyond the sphere on -X are rejected by the box test")
	{
		checkSphereLineCase(
			Vector3(-11.0f, 0.0f, 0.0f), Vector3(-10.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("the box test rejects each of the six axis directions")
	{
		checkSphereLineCase(Vector3(0.0f, 10.0f, 0.0f), Vector3(0.0f, 11.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
		checkSphereLineCase(Vector3(0.0f, -11.0f, 0.0f), Vector3(0.0f, -10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
		checkSphereLineCase(Vector3(0.0f, 0.0f, 10.0f), Vector3(0.0f, 0.0f, 11.0f), Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
		checkSphereLineCase(Vector3(0.0f, 0.0f, -11.0f), Vector3(0.0f, 0.0f, -10.0f), Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("only one endpoint outside the box is not enough to reject")
	{
		// The box test is a conjunction, so a straddling segment survives to the
		// discriminant.
		checkSphereLineCase(
			Vector3(-5.0f, 0.0f, 0.0f), Vector3(10.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);
	}

	SECTION("an endpoint exactly on the box boundary is not rejected")
	{
		// The comparisons are strict, so x == x3 + r stays inside the box.
		checkSphereLineCase(
			Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 1.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);
	}

	SECTION("a degenerate radius of zero is accepted at the exact contact point")
	{
		checkSphereLineCase(
			Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 0.0f, true);
	}

	SECTION("a degenerate radius of zero misses a point away from the centre")
	{
		checkSphereLineCase(
			Vector3(0.0f, 0.5f, 0.0f), Vector3(0.5f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 0.0f, false);
	}

	SECTION("a degenerate radius of zero still applies the box test")
	{
		checkSphereLineCase(
			Vector3(1.0f, 0.0f, 0.0f), Vector3(2.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 0.0f, false);
	}

	SECTION("a zero length segment inside the sphere always reports a hit")
	{
		// a == 0 collapses the discriminant to b*b, which is never negative, so
		// the answer depends only on whether the point escaped the box test.
		checkSphereLineCase(
			Vector3(0.1f, 0.1f, 0.1f), Vector3(0.1f, 0.1f, 0.1f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);

		checkSphereLineCase(
			Vector3(5.0f, 0.0f, 0.0f), Vector3(5.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("negative coordinates hit and miss correctly")
	{
		checkSphereLineCase(
			Vector3(-5.0f, 0.0f, 0.0f), Vector3(5.0f, 0.0f, 0.0f),
			Vector3(-1.0f, 0.0f, 0.0f), 1.0f, true);

		checkSphereLineCase(
			Vector3(-5.0f, 4.0f, 0.0f), Vector3(5.0f, 4.0f, 0.0f),
			Vector3(-1.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("large magnitudes hit and miss correctly")
	{
		checkSphereLineCase(
			Vector3(-500.0f, 0.0f, 0.0f), Vector3(500.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);

		checkSphereLineCase(
			Vector3(-500.0f, 300.0f, 0.0f), Vector3(500.0f, 300.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("a huge radius swallows the line")
	{
		checkSphereLineCase(
			Vector3(-5.0f, 3.0f, 0.0f), Vector3(5.0f, 3.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0e3f, true);
	}

	SECTION("the two overloads agree on every case above")
	{
		checkSphereLineCase(
			Vector3(-5.0f, 0.0f, 0.0f), Vector3(5.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);
		checkSphereLineCase(
			Vector3(-5.0f, 3.0f, 0.0f), Vector3(5.0f, 3.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
	}

	SECTION("a hit followed by a miss followed by a hit still answers correctly")
	{
		// The discriminant scratch is recomputed on every call, so the second
		// answer cannot be inherited from the first.
		checkSphereLineCase(
			Vector3(-5.0f, 0.0f, 0.0f), Vector3(5.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);
		checkSphereLineCase(
			Vector3(-5.0f, 3.0f, 0.0f), Vector3(5.0f, 3.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, false);
		checkSphereLineCase(
			Vector3(-5.0f, 0.0f, 0.0f), Vector3(5.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), 1.0f, true);
	}

	SECTION("the two overloads do not disturb each other")
	{
		Vector3 p1(-5.0f, 0.0f, 0.0f);
		Vector3 p2(5.0f, 0.0f, 0.0f);
		Vector3 p3(0.0f, 0.0f, 0.0f);
		Vector3 miss1(-5.0f, 3.0f, 0.0f);
		Vector3 miss2(5.0f, 3.0f, 0.0f);
		Vector3 miss3(0.0f, 0.0f, 0.0f);
		float r = 1.0f;
		for (int i = 0; i < 3; i++)
		{
			REQUIRE(sphere_line_intersection(&p1, &p2, &p3, &r));
			REQUIRE(sphere_line_intersection(-5.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
			REQUIRE_FALSE(sphere_line_intersection(&miss1, &miss2, &miss3, &r));
		}
	}

	SECTION("the inputs are not modified by either overload")
	{
		Vector3 p1(-5.0f, 0.0f, 0.0f);
		Vector3 p2(5.0f, 0.0f, 0.0f);
		Vector3 p3(0.0f, 0.0f, 0.0f);
		float r = 1.0f;
		REQUIRE(sphere_line_intersection(&p1, &p2, &p3, &r));

		REQUIRE(p1.x == -5.0f);
		REQUIRE(p1.y == 0.0f);
		REQUIRE(p1.z == 0.0f);
		REQUIRE(p2.x == 5.0f);
		REQUIRE(p3.x == 0.0f);
		REQUIRE(r == 1.0f);
	}
}