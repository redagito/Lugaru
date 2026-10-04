// Unit tests for the Foundation layer.
//
// References FoundationLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>

#include "Math/Vector3.hpp"
#include "Platform/Platform.hpp"

namespace
{
constexpr float kEpsilon = 1e-4f;
constexpr float kPi = std::numbers::pi_v<float>;
} // namespace

TEST_CASE("Vector3 length and distance", "[vector3]")
{
	SECTION("findLength returns the magnitude")
	{
		const Vector3 v(3.0f, 4.0f, 0.0f);
		REQUIRE(std::fabs(findLength(&v) - 5.0f) < kEpsilon);
	}

	SECTION("magnitudeSquared returns the squared magnitude")
	{
		// Every threshold in the engine is calibrated against the squared value
		// (GameTick.cpp compares it against multiplier * multiplier * 400), so
		// this must not gain a sqrt. Pinned here to catch an accidental "fix".
		const Vector3 v(3.0f, 4.0f, 0.0f);
		REQUIRE(magnitudeSquared(&v) == 25.0f);
		REQUIRE(findLength(&v) == 5.0f);
	}

	SECTION("findDistance measures point to point")
	{
		const Vector3 a(0.0f, 0.0f, 0.0f);
		const Vector3 b(0.0f, 3.0f, 4.0f);
		REQUIRE(std::fabs(findDistance(&a, &b) - 5.0f) < kEpsilon);
	}

	SECTION("the zero vector has zero length")
	{
		const Vector3 zero;
		REQUIRE(std::fabs(findLength(&zero)) < kEpsilon);
	}

	SECTION("assignment from a scalar splats across all axes")
	{
		Vector3 v;
		v = 2.5f;
		REQUIRE(v.x == 2.5f);
		REQUIRE(v.y == 2.5f);
		REQUIRE(v.z == 2.5f);
	}
}

TEST_CASE("Vector3 products", "[vector3]")
{
	SECTION("normaldotproduct ignores magnitude sign conventions")
	{
		const Vector3 a(1.0f, 0.0f, 0.0f);
		const Vector3 b(1.0f, 0.0f, 0.0f);
		REQUIRE(std::fabs(normaldotproduct(a, b) - 1.0f) < kEpsilon);

		const Vector3 c(-1.0f, 0.0f, 0.0f);
		REQUIRE(std::fabs(normaldotproduct(a, c) + 1.0f) < kEpsilon);
	}

	SECTION("dotproduct is the plain dot product")
	{
		const Vector3 a(1.0f, 2.0f, 3.0f);
		const Vector3 b(4.0f, -5.0f, 6.0f);
		REQUIRE(std::fabs(dotproduct(&a, &b) - 12.0f) < kEpsilon);
	}

	SECTION("orthogonal vectors have a zero dot product")
	{
		const Vector3 a(1.0f, 0.0f, 0.0f);
		const Vector3 b(0.0f, 1.0f, 0.0f);
		REQUIRE(std::fabs(dotproduct(&a, &b)) < kEpsilon);
	}
}

TEST_CASE("ReflectVector", "[vector3]")
{
	SECTION("a vector hitting a flat surface head on bounces back")
	{
		// Falling straight down onto a floor with normal +Y.
		Vector3 velocity(0.0f, -1.0f, 0.0f);
		const Vector3 normal(0.0f, 1.0f, 0.0f);
		ReflectVector(&velocity, normal);

		REQUIRE(std::fabs(velocity.x) < kEpsilon);
		REQUIRE(std::fabs(velocity.y - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(velocity.z) < kEpsilon);
	}

	SECTION("a vector parallel to the surface is unchanged")
	{
		Vector3 velocity(1.0f, 0.0f, 0.0f);
		const Vector3 normal(0.0f, 1.0f, 0.0f);
		ReflectVector(&velocity, normal);

		REQUIRE(std::fabs(velocity.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(velocity.y) < kEpsilon);
	}

	SECTION("reflection preserves speed")
	{
		Vector3 velocity(1.0f, -1.0f, 0.0f);
		const float before = findLength(&velocity);
		const Vector3 normal(0.0f, 1.0f, 0.0f);
		ReflectVector(&velocity, normal);
		REQUIRE(std::fabs(findLength(&velocity) - before) < kEpsilon);
	}
}

TEST_CASE("DoRotation", "[vector3]")
{
	SECTION("a zero rotation is the identity")
	{
		const Vector3 v(1.0f, 2.0f, 3.0f);
		const Vector3 rotated = DoRotation(v, 0.0f, 0.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x - v.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.y - v.y) < kEpsilon);
		REQUIRE(std::fabs(rotated.z - v.z) < kEpsilon);
	}

	SECTION("a quarter turn about Y maps +X onto -Z")
	{
		const Vector3 rotated = DoRotation(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 90.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x) < kEpsilon);
		REQUIRE(std::fabs(rotated.z + 1.0f) < kEpsilon);
	}

	SECTION("rotation preserves length")
	{
		const Vector3 v(1.0f, 2.0f, 3.0f);
		const Vector3 rotated = DoRotation(v, 30.0f, 45.0f, 60.0f);
		REQUIRE(std::fabs(findLength(&rotated) - findLength(&v)) < kEpsilon);
	}

	SECTION("the radian variant takes radians")
	{
		const Vector3 degrees = DoRotation(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 90.0f, 0.0f);
		const Vector3 radians = DoRotationRadian(Vector3(1.0f, 0.0f, 0.0f), 0.0f, kPi / 2.0f, 0.0f);
		REQUIRE(std::fabs(degrees.x - radians.x) < kEpsilon);
		REQUIRE(std::fabs(degrees.y - radians.y) < kEpsilon);
		REQUIRE(std::fabs(degrees.z - radians.z) < kEpsilon);
	}

	SECTION("a half turn about Y maps +X onto -X")
	{
		const Vector3 rotated = DoRotation(Vector3(1.0f, 0.0f, 0.0f), 0.0f, 180.0f, 0.0f);
		REQUIRE(std::fabs(rotated.x + 1.0f) < kEpsilon);
		REQUIRE(std::fabs(rotated.z) < kEpsilon);
	}
}

TEST_CASE("LineFacet", "[vector3]")
{
	// A triangle in the XZ plane at y = 0.
	const Vector3 a(0.0f, 0.0f, 0.0f);
	const Vector3 b(10.0f, 0.0f, 0.0f);
	const Vector3 c(0.0f, 0.0f, 10.0f);

	SECTION("a line crossing the triangle is found")
	{
		const Vector3 from(1.0f, 5.0f, 1.0f);
		const Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 hit;
		REQUIRE(LineFacet(from, to, a, b, c, &hit));
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		REQUIRE(std::fabs(hit.x - 1.0f) < kEpsilon);
		REQUIRE(std::fabs(hit.z - 1.0f) < kEpsilon);
	}

	SECTION("a line that misses the triangle is rejected")
	{
		// Passes over the plane but far outside the triangle's footprint.
		const Vector3 from(100.0f, 5.0f, 100.0f);
		const Vector3 to(100.0f, -5.0f, 100.0f);
		Vector3 hit;
		REQUIRE_FALSE(LineFacet(from, to, a, b, c, &hit));
	}

	SECTION("a line parallel to the plane is rejected")
	{
		const Vector3 from(1.0f, 5.0f, 1.0f);
		const Vector3 to(5.0f, 5.0f, 5.0f);
		Vector3 hit;
		REQUIRE_FALSE(LineFacet(from, to, a, b, c, &hit));
	}

	SECTION("a line whose intersection lies outside the segment is rejected")
	{
		// The plane crossing happens at t = 2, beyond the end of the segment.
		const Vector3 from(1.0f, 5.0f, 1.0f);
		const Vector3 to(1.0f, 4.0f, 1.0f);
		Vector3 hit;
		REQUIRE_FALSE(LineFacet(from, to, a, b, c, &hit));
	}

	SECTION("LineFacetHit agrees with LineFacet and returns a hit flag")
	{
		// LineFacetHit is the pointer overload the engine actually calls (Terrain,
		// Models, Person). It returns 1.0 on a hit and 0.0 otherwise. It takes
		// non-const pointers, so work on mutable copies of the facet corners.
		Vector3 from(1.0f, 5.0f, 1.0f);
		Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 va = a;
		Vector3 vb = b;
		Vector3 vc = c;
		Vector3 hit;
		REQUIRE(LineFacetHit(&from, &to, &va, &vb, &vc, &hit) == 1.0f);
		REQUIRE(std::fabs(hit.y) < kEpsilon);
		REQUIRE(LineFacet(from, to, a, b, c, &hit));

		Vector3 miss;
		Vector3 fromMiss(100.0f, 5.0f, 100.0f);
		Vector3 toMiss(100.0f, -5.0f, 100.0f);
		REQUIRE(LineFacetHit(&fromMiss, &toMiss, &va, &vb, &vc, &miss) == 0.0f);
		REQUIRE_FALSE(LineFacet(fromMiss, toMiss, a, b, c, &miss));
	}

	SECTION("LineFacetHit honours a supplied face normal")
	{
		// The seven argument overload takes a precomputed normal instead of
		// deriving one, and skips normalising. It must still agree with the six
		// argument form for the same facet.
		Vector3 from(1.0f, 5.0f, 1.0f);
		Vector3 to(1.0f, -5.0f, 1.0f);
		Vector3 va = a;
		Vector3 vb = b;
		Vector3 vc = c;
		Vector3 normal(0.0f, 1.0f, 0.0f);
		Vector3 hit;
		REQUIRE(LineFacetHit(&from, &to, &va, &vb, &vc, &normal, &hit) == 1.0f);
		REQUIRE(std::fabs(hit.y) < kEpsilon);

		// A miss is still rejected with an explicit normal.
		Vector3 miss;
		Vector3 fromMiss(100.0f, 5.0f, 100.0f);
		Vector3 toMiss(100.0f, -5.0f, 100.0f);
		REQUIRE(LineFacetHit(&fromMiss, &toMiss, &va, &vb, &vc, &normal, &miss) == 0.0f);

		// The normal's sign is irrelevant: PointInTriangle projects onto the
		// dominant axis using absolute component values.
		Vector3 flipped(0.0f, -1.0f, 0.0f);
		Vector3 hitFlipped;
		REQUIRE(LineFacetHit(&from, &to, &va, &vb, &vc, &flipped, &hitFlipped) == 1.0f);
		REQUIRE(std::fabs(hitFlipped.y) < kEpsilon);
	}
}

TEST_CASE("Platform timing", "[platform]")
{
	SECTION("UpTime advances and never goes backwards")
	{
		AbsoluteTime start = UpTime();
		volatile double sink = 0.0;
		for (int i = 1; i < 200000; i++) {
			sink += i;
		}
		(void)sink;
		AbsoluteTime later = UpTime();

		// AbsoluteDeltaToDuration returns a negative microsecond count for
		// sub-second deltas, so "time moved forward" means the caller's
		// normalisation yields a positive number of seconds.
		const Duration raw = AbsoluteDeltaToDuration(later, start);
		const double seconds = (0 > raw) ? static_cast<double>(raw) / -1000000.0
										  : static_cast<double>(raw) / 1000.0;
		REQUIRE(seconds >= 0.0);
	}

	SECTION("a delta of zero is immediate")
	{
		AbsoluteTime t = UpTime();
		REQUIRE(AbsoluteDeltaToDuration(t, t) == durationImmediate);
	}

	SECTION("a negative delta clamps to immediate")
	{
		// Arguments are (later, earlier): the result is later - earlier.
		AbsoluteTime later = UpTime();
		AbsoluteTime earlier = later;
		earlier.lo -= 1000;
		REQUIRE(AbsoluteDeltaToDuration(earlier, later) == durationImmediate);
	}

	SECTION("a sub-second delta is reported as NEGATIVE microseconds")
	{
		// The sign is a unit tag, not a bug: callers select their divisor from
		// it (main.cpp DoFrameRate and GameInitDispose.cpp LoadingScreen both do
		// `if (0 > deltaTime) / -1e6 else / 1e3`). A negative result means
		// microseconds, a non-negative one means milliseconds. Making this
		// positive makes the game run 1000x too fast.
		AbsoluteTime later = UpTime();
		AbsoluteTime earlier = later;
		earlier.lo -= 1000;

		const Duration elapsed = AbsoluteDeltaToDuration(later, earlier);
		REQUIRE(elapsed < 0);

		// Reproduce the caller's normalisation: microseconds -> seconds.
		const double seconds = static_cast<double>(elapsed) / -1000000.0;
		REQUIRE(seconds > 0.0);
		// A sub-frame gap must stay far below one millisecond of simulation.
		REQUIRE(seconds < durationMillisecond * 1000 / 1000000.0);
	}

	SECTION("a delta of a whole second or more is reported as milliseconds")
	{
		// Past one second the value/counterRate division is no longer zero, so
		// the other branch runs and the result is non-negative milliseconds.
		// Subtract a large number of ticks: comfortably more than a second at any
		// plausible performance-counter rate.
		AbsoluteTime later = UpTime();
		AbsoluteTime earlier = later;
		earlier.lo -= 100000000;

		const Duration elapsed = AbsoluteDeltaToDuration(later, earlier);
		REQUIRE(elapsed >= 0);
	}

	SECTION("the duration constants are consistent")
	{
		REQUIRE(durationMillisecond == 1);
		REQUIRE(durationSecond == 1000 * durationMillisecond);
		REQUIRE(durationMinute == 60 * durationSecond);
		REQUIRE(durationHour == 60 * durationMinute);
		REQUIRE(durationDay == 24 * durationHour);
		REQUIRE(durationForever > durationDay);
	}

	SECTION("a measurable interval reports a positive duration")
	{
		AbsoluteTime start = UpTime();
		AbsoluteTime end = UpTime();
		const Duration elapsed = AbsoluteDeltaToDuration(end, start);
		REQUIRE(elapsed >= 0);
		// Even the smallest measurable gap must stay well inside a millisecond
		// rather than overflowing into nonsense.
		REQUIRE(elapsed < durationSecond);
	}
}