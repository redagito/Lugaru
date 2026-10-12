// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// A regression test for an out-of-bounds read on the terrain's heightmap.
//
// Terrain::MakeDecal converts a world position into patch indices with a plain
// divide and no clamping, then Decal::Decal indexes heightmap[whichx + 1]
// [whichy + 1] with them. heightmap is 257x257, so an index outside roughly
// [-1, 255] reads past the array. Any decal placed by something that has
// travelled outside the terrain does exactly that - a ragdoll that has been
// thrown, or a body below the floor.
//
// This is the third out-of-bounds found in the same family: UpdateVertexArray
// indexing [i + stepsize] on 256-wide arrays, the unguarded patchobjects index,
// and now this. Two of the three only became fatal when those arrays moved onto
// the heap, because before then the overflow landed silently in a neighbouring
// member. Expect more of this shape; the pattern is "a world coordinate is used
// as an array index without being clamped".
//
// The test asserts the placement survives rather than reading out of bounds. It
// does not assert where the decal ends up, because a decal outside the terrain
// legitimately belongs to no patch.

#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "Environment/Terrain.hpp"
#include "Graphic/Decal.hpp"
#include "Utils/Folders.hpp"

namespace
{

// Enough of a terrain to index into. The heightmap is what gets read out of
// bounds, so it has to be loaded for the bug to be reachable at all.
void loadTestTerrain(Terrain& terrain)
{
    terrain.scale = 3 * 1 * 2;
    // Fallback when the shipped heightmap is missing: without it the arrays are
    // zero-sized and size stays 0, which is a different failure from the one
    // this test is about.
    if (!Folders::file_exists(Folders::getResourcePath("Textures/HeightMap.png"))) {
        return;
    }
    terrain.load("Textures/HeightMap.png", 0, []() {});
}

} // namespace

TEST_CASE("a decal can be placed from anywhere without leaving the terrain", "[terrain][decals]")
{
    Terrain terrain;
    loadTestTerrain(terrain);
    if (terrain.size == 0) {
        SKIP("the shipped heightmap is not available in this working directory");
    }

    SECTION("a decal well inside the terrain is placed")
    {
        const Vector3 inside(30, 0, 30);
        terrain.MakeDecal(blooddecal, inside, 2, .5f, 0, 0);
        // Nothing asserts on the outcome: a decal that was placed is fine, and
        // one that was rejected is fine. The point is that it does not crash.
        SUCCEED();
    }

    SECTION("a decal from far outside the terrain does not read past it")
    {
        // These are the coordinates that produce an out-of-range patch index.
        const float limit = terrain.size * terrain.scale;
        for (const Vector3& outside : {
                 Vector3(limit * 20, 0, limit * 20),
                 Vector3(-limit * 20, 0, -limit * 20),
                 Vector3(limit * 20, 0, -limit * 20),
                 Vector3(1e6f, 0, 1e6f),
             }) {
            INFO("placing a decal at " << outside.x << ", " << outside.z);
            // Before the clamp this read past heightmap. This call surviving is
            // the assertion.
            terrain.MakeDecal(blooddecal, outside, 2, .5f, 0, 0);
        }
        SUCCEED();
    }
}
