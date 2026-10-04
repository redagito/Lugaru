// Unit tests for the Foundation layer.
//
// References FoundationLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "Math/Frustum.hpp"
#include "Math/Math.h"
#include "Math/Vector3.hpp"
#include "Utils/Folders.hpp"

namespace
{

constexpr float kEpsilon = 1e-5f;

// Column-major perspective projection, matching the layout SetFrustum expects.
void makePerspective(float proj[16], float fovyDegrees, float aspect, float zNear, float zFar)
{
	const float f = 1.0f / std::tan(fovyDegrees * 3.14159265358979f / 360.0f);
	for (int i = 0; i < 16; i++) {
		proj[i] = 0.0f;
	}
	proj[0] = f / aspect;
	proj[5] = f;
	proj[10] = (zFar + zNear) / (zNear - zFar);
	proj[11] = -1.0f;
	proj[14] = (2.0f * zFar * zNear) / (zNear - zFar);
}

// View matrix for a camera at (x, y, z). OpenGL cameras look down -Z, so the
// translation is the negated camera position and points with a negative
// view-space z are in front of the camera.
void makeTranslation(float mv[16], float x, float y, float z)
{
	for (int i = 0; i < 16; i++) {
		mv[i] = 0.0f;
	}
	mv[0] = 1.0f;
	mv[5] = 1.0f;
	mv[10] = 1.0f;
	mv[12] = -x;
	mv[13] = -y;
	mv[14] = -z;
	mv[15] = 1.0f;
}

Frustum makeFrustumAt(float camX, float camY, float camZ)
{
	float proj[16];
	float mv[16];
	makePerspective(proj, 90.0f, 1.0f, 1.0f, 100.0f);
	makeTranslation(mv, camX, camY, camZ);

	Frustum f;
	f.SetFrustum(proj, mv);
	return f;
}

} // namespace

TEST_CASE("Vector3 arithmetic", "[vector3]")
{
	const Vector3 a(1.0f, 2.0f, 3.0f);
	const Vector3 b(4.0f, 5.0f, 6.0f);

	SECTION("addition and subtraction are componentwise")
	{
		const Vector3 sum = a + b;
		REQUIRE(sum.x == 1.0f + 4.0f);
		REQUIRE(sum.y == 2.0f + 5.0f);
		REQUIRE(sum.z == 3.0f + 6.0f);

		const Vector3 diff = b - a;
		REQUIRE(diff.x == 3.0f);
		REQUIRE(diff.y == 3.0f);
		REQUIRE(diff.z == 3.0f);
	}

	SECTION("compound assignment")
	{
		Vector3 v(1.0f, 1.0f, 1.0f);
		v += b;
		REQUIRE(v.x == 5.0f);
		REQUIRE(v.y == 6.0f);
		REQUIRE(v.z == 7.0f);

		v -= a;
		REQUIRE(v.x == 4.0f);
		REQUIRE(v.y == 4.0f);
		REQUIRE(v.z == 4.0f);

		v *= 2.0f;
		REQUIRE(v.x == 8.0f);
		REQUIRE(v.y == 8.0f);
		REQUIRE(v.z == 8.0f);

		v /= 4.0f;
		REQUIRE(v.x == 2.0f);
		REQUIRE(v.y == 2.0f);
		REQUIRE(v.z == 2.0f);
	}

	SECTION("scalar and vector multiplication")
	{
		const Vector3 scaled = a * 2.0f;
		REQUIRE(scaled.x == 2.0f);
		REQUIRE(scaled.y == 4.0f);
		REQUIRE(scaled.z == 6.0f);

		const Vector3 hadamard = a * b;
		REQUIRE(hadamard.x == 4.0f);
		REQUIRE(hadamard.y == 10.0f);
		REQUIRE(hadamard.z == 18.0f);
	}

	SECTION("equality")
	{
		REQUIRE(a == Vector3(1.0f, 2.0f, 3.0f));
		REQUIRE_FALSE(a == b);
	}
}

TEST_CASE("Vector3 geometric helpers", "[vector3]")
{
	SECTION("Normalise produces a unit vector")
	{
		Vector3 v(3.0f, 0.0f, 4.0f);
		Normalise(&v);
		REQUIRE(std::fabs(findLength(&v) - 1.0f) < kEpsilon);
	}

	SECTION("Normalise leaves the zero vector alone")
	{
		// Guards against a division by zero producing NaN.
		Vector3 v(0.0f, 0.0f, 0.0f);
		Normalise(&v);
		REQUIRE(v.x == 0.0f);
		REQUIRE(v.y == 0.0f);
		REQUIRE(v.z == 0.0f);
	}

	SECTION("distsq measures the full 3D distance")
	{
		const Vector3 a(0.0f, 0.0f, 0.0f);
		const Vector3 b(3.0f, 4.0f, 0.0f);
		REQUIRE(std::fabs(distsq(&a, &b) - 25.0f) < kEpsilon);
	}

	SECTION("distsqflat ignores the vertical axis")
	{
		const Vector3 a(0.0f, 0.0f, 0.0f);
		const Vector3 b(3.0f, 100.0f, 4.0f);
		REQUIRE(std::fabs(distsqflat(&a, &b) - 25.0f) < kEpsilon);
		// The full 3D distance does account for the height difference.
		REQUIRE(distsq(&a, &b) > distsqflat(&a, &b));
	}

	SECTION("CrossProduct follows the right-hand rule")
	{
		Vector3 result;
		CrossProduct(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), &result);
		REQUIRE(std::fabs(result.x) < kEpsilon);
		REQUIRE(std::fabs(result.y) < kEpsilon);
		REQUIRE(std::fabs(result.z - 1.0f) < kEpsilon);
	}

	SECTION("PointInTriangle accepts interior points and rejects exterior ones")
	{
		const Vector3 normal(0.0f, 1.0f, 0.0f);
		Vector3 p1(0.0f, 0.0f, 0.0f);
		Vector3 p2(1.0f, 0.0f, 0.0f);
		Vector3 p3(0.0f, 0.0f, 1.0f);

		Vector3 inside(0.25f, 0.0f, 0.25f);
		REQUIRE(PointInTriangle(&inside, normal, &p1, &p2, &p3));

		Vector3 outside(5.0f, 0.0f, 5.0f);
		REQUIRE_FALSE(PointInTriangle(&outside, normal, &p1, &p2, &p3));
	}

	SECTION("DistancePointLine measures the perpendicular distance")
	{
		// Line along +X at the origin; the point sits 2 units above it.
		Vector3 point(0.0f, 2.0f, 0.0f);
		Vector3 start(0.0f, 0.0f, 0.0f);
		Vector3 end(10.0f, 0.0f, 0.0f);
		float distance = -1.0f;
		Vector3 intersection;

		REQUIRE(DistancePointLine(&point, &start, &end, &distance, &intersection));
		REQUIRE(std::fabs(distance - 2.0f) < kEpsilon);
	}
}

TEST_CASE("Math helpers", "[math]")
{
	SECTION("sq squares its argument")
	{
		REQUIRE(sq(4.0f) == 16.0f);
		REQUIRE(sq(-4.0f) == 16.0f);
		REQUIRE(sq(0.0f) == 0.0f);
	}

	SECTION("roughDirection maps the cardinal axes to expected angles")
	{
		REQUIRE(std::fabs(roughDirection(Vector3(0.0f, 0.0f, 1.0f))) < kEpsilon);
		REQUIRE(std::fabs(roughDirection(Vector3(0.0f, 0.0f, -1.0f)) - 180.0f) < kEpsilon);
		REQUIRE(std::fabs(roughDirection(Vector3(1.0f, 0.0f, 0.0f)) - 90.0f) < kEpsilon);
		REQUIRE(std::fabs(roughDirection(Vector3(-1.0f, 0.0f, 0.0f)) + 90.0f) < kEpsilon);
	}

	SECTION("roughDirection ignores magnitude and pitch")
	{
		REQUIRE(std::fabs(roughDirection(Vector3(0.0f, 0.0f, 5.0f))) < kEpsilon);
	}

	SECTION("pitchOf maps the vertical axis to expected angles")
	{
		REQUIRE(std::fabs(pitchOf(Vector3(0.0f, 0.0f, 1.0f))) < kEpsilon);
		REQUIRE(std::fabs(pitchOf(Vector3(0.0f, 1.0f, 0.0f)) + 90.0f) < kEpsilon);
		REQUIRE(std::fabs(pitchOf(Vector3(0.0f, -1.0f, 0.0f)) - 90.0f) < kEpsilon);
	}

	SECTION("stepTowardf clamps instead of overshooting")
	{
		REQUIRE(stepTowardf(0.0f, 10.0f, 3.0f) == 3.0f);
		REQUIRE(stepTowardf(0.0f, 2.0f, 3.0f) == 2.0f);
		REQUIRE(stepTowardf(10.0f, 0.0f, 3.0f) == 7.0f);
		REQUIRE(stepTowardf(10.0f, 9.0f, 3.0f) == 9.0f);
	}

	SECTION("stepTowardf is stable once it reaches the target")
	{
		float value = 5.0f;
		value = stepTowardf(value, 0.0f, 1.0f);
		REQUIRE(value == 4.0f);
		value = stepTowardf(value, 0.0f, 1.0f);
		value = stepTowardf(value, 0.0f, 1.0f);
		value = stepTowardf(value, 0.0f, 1.0f);
		value = stepTowardf(value, 0.0f, 1.0f);
		REQUIRE(value == 0.0f);
	}
}

TEST_CASE("Frustum culling", "[frustum]")
{
	// Camera at the origin looking down -Z with a 90 degree FOV, 1..100 depth range.
	const Frustum f = makeFrustumAt(0.0f, 0.0f, 0.0f);

	SECTION("a point straight ahead is fully inside")
	{
		REQUIRE(f.SphereInFrustum(0.0f, 0.0f, -10.0f, 0.5f) == 2);
	}

	SECTION("a point behind the camera is fully outside")
	{
		REQUIRE(f.SphereInFrustum(0.0f, 0.0f, 10.0f, 0.5f) == 0);
	}

	SECTION("a point beyond the far plane is fully outside")
	{
		REQUIRE(f.SphereInFrustum(0.0f, 0.0f, -500.0f, 0.5f) == 0);
	}

	SECTION("a point nearer than the near plane is fully outside")
	{
		REQUIRE(f.SphereInFrustum(0.0f, 0.0f, -0.1f, 0.5f) == 0);
	}

	SECTION("a point off to the side is rejected")
	{
		REQUIRE(f.SphereInFrustum(100.0f, 0.0f, -10.0f, 0.5f) == 0);
	}

	SECTION("a point far above the camera is rejected")
	{
		REQUIRE(f.SphereInFrustum(0.0f, 100.0f, -10.0f, 0.5f) == 0);
	}

	SECTION("a small cube ahead of the camera is fully inside")
	{
		REQUIRE(f.CubeInFrustum(0.0f, 0.0f, -10.0f, 0.5f) == 2);
	}

	SECTION("a cube behind the camera is fully outside")
	{
		REQUIRE(f.CubeInFrustum(0.0f, 0.0f, 10.0f, 0.5f) == 0);
	}

	SECTION("a cube centred on the near plane is partially visible")
	{
		// The near plane sits one unit in front of the camera, so a cube of half
		// size 0.5 centred there is cut by it: visible, but not fully contained.
		REQUIRE(f.CubeInFrustum(0.0f, 0.0f, -1.0f, 0.5f) == 1);
	}

	SECTION("a cube straddling a side plane is partially visible")
	{
		// With a 90 degree FOV the right plane passes through the view axis, so
		// an off-centre cube is clipped by it.
		REQUIRE(f.CubeInFrustum(1.0f, 0.0f, -2.0f, 0.5f) == 1);
	}

	SECTION("the frustum follows the camera")
	{
		// From the origin, the point at z = 10 is behind the camera.
		REQUIRE(f.SphereInFrustum(0.0f, 0.0f, 10.0f, 0.5f) == 0);

		// Move the camera to z = 20 and the same point is now 10 units ahead.
		const Frustum moved = makeFrustumAt(0.0f, 0.0f, 20.0f);
		REQUIRE(moved.SphereInFrustum(0.0f, 0.0f, 10.0f, 0.5f) == 2);
	}

	SECTION("the far plane moves with the camera too")
	{
		// 120 units in front of a camera at z = 20 is 140 units of view depth,
		// past the 100 unit far plane.
		const Frustum moved = makeFrustumAt(0.0f, 0.0f, 20.0f);
		REQUIRE(moved.SphereInFrustum(0.0f, 0.0f, -100.0f, 0.5f) == 2);
		REQUIRE(moved.SphereInFrustum(0.0f, 0.0f, -200.0f, 0.5f) == 0);
	}
}

TEST_CASE("Folders", "[folders]")
{
	SECTION("getResourcePath joins onto the data directory")
	{
		REQUIRE(Folders::getResourcePath("Models/wolf.obj") == std::string(DATA_DIR) + "/Models/wolf.obj");
	}

	SECTION("makeDirectory is idempotent")
	{
		const std::string path = Folders::getScreenshotDir();
		REQUIRE(Folders::makeDirectory(path));
		REQUIRE(Folders::makeDirectory(path));
	}

	SECTION("file_exists detects missing files")
	{
		REQUIRE_FALSE(Folders::file_exists("this-file-should-not-exist-lugaru"));
	}

	SECTION("openMandatoryFile throws for missing files")
	{
		REQUIRE_THROWS_AS(Folders::openMandatoryFile("this-file-should-not-exist-lugaru", "rb"), FileNotFoundException);
	}
}