// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// Tests for the terrain's query surface: getHeight, getHeightByTile, getOpacity,
// getNormal and patchFor.
//
// This is the layer the whole game rests on - every character stands on it, every
// object is placed against it, and lineTerrain is the primitive that decides
// whether something is walking on the ground or falling through it. Until now it
// had no direct tests at all: the levels load, and WorldPlacementTest checks the
// objects ended up somewhere sensible, but nothing pinned the queries themselves.
//
// Two of these are worth their keep specifically:
//
//   patchFor is the clamp added during the memory-safety pass. It converts a world
//   coordinate into a patch index, and before it clamped, a coordinate outside the
//   terrain produced an index of any magnitude and indexed patchobjects with it.
//   That is one of the bugs this family produced, so its clamping is pinned here
//   rather than left to be re-broken by a refactor.
//
//   getHeightByTile now throws on an out-of-range tile instead of reading past the
//   end of the heightmap. The negative-side guards were always there; the positive
//   side never was, and it is covered here.

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <memory>
#include <string>

#include "Environment/Terrain.hpp"

namespace
{

// The terrain every level loads, so the queries run against real data rather than
// a synthetic grid: the bilinear paths, the triangle walk and the normal buffers
// are all only exercised by a heightmap that has been through load and
// CalculateNormals.
Terrain& loadedTerrain()
{
    static Terrain terrain;
    static bool loaded = false;
    if (!loaded) {
        REQUIRE(terrain.load("Textures/HeightMap.png", 0, []() {}));
        terrain.CalculateNormals();
        loaded = true;
    }
    // The world scale the game runs at. Without it the terrain spans 256 units
    // instead of 1536 and nothing in the game lands on it.
    terrain.scale = 3 * 1 * 2;
    return terrain;
}

// A point comfortably inside the terrain, well away from the clamped edge where
// every query returns zero.
float wellInsideCoordinate()
{
    return 100.f;
}

} // namespace

TEST_CASE("patchFor clamps a world coordinate into a patch index", "[terrain][query]")
{
    Terrain& terrain = loadedTerrain();

    // size/subdivision is integer division, and the patch stride in world units
    // is that times the scale.
    const float patchStride = terrain.size / subdivision * terrain.scale;
    REQUIRE(patchStride > 0);

    SECTION("a negative coordinate clamps to the first patch")
    {
        REQUIRE(terrain.patchFor(-1.f) == 0);
        REQUIRE(terrain.patchFor(-100000.f) == 0);
    }

    SECTION("zero maps to the first patch")
    {
        REQUIRE(terrain.patchFor(0.f) == 0);
    }

    SECTION("a coordinate past the end of the world clamps to the last patch")
    {
        // This is the case that used to produce an out-of-range index. The patch
        // table is 64 wide, so anything the terrain could not hold landed outside
        // it and was used to index patchobjects.
        const float onePastTheEnd = patchStride * (subdivision + 1);
        REQUIRE(terrain.patchFor(onePastTheEnd) == subdivision - 1);
        REQUIRE(terrain.patchFor(1e9f) == subdivision - 1);
    }

    SECTION("an interior coordinate maps to the patch that contains it")
    {
        for (int patch = 0; patch < subdivision; patch++) {
            const float coordinate = patchStride * patch + patchStride / 2;
            INFO("coordinate " << coordinate << " should be in patch " << patch);
            REQUIRE(terrain.patchFor(coordinate) == patch);
        }
    }

    SECTION("patchFor never decreases as the coordinate grows")
    {
        float previous = terrain.patchFor(0.f);
        for (float coordinate = 0; coordinate < patchStride * subdivision * 2; coordinate += patchStride / 7) {
            const float current = terrain.patchFor(coordinate);
            REQUIRE(current >= previous);
            previous = current;
        }
    }

    SECTION("the result is always a readable patch index")
    {
        // Not merely "in range" in the abstract: the patch tables patchobjects,
        // numtris and textureness are all subdivision wide, so an index outside
        // that range cannot be used to read them at all.
        for (const float coordinate : {-1e9f, -1.f, 0.f, 1.f, patchStride, patchStride * subdivision - 1, patchStride * subdivision, 1e9f}) {
            const int patch = terrain.patchFor(coordinate);
            INFO("coordinate " << coordinate << " gave patch " << patch);
            REQUIRE(patch >= 0);
            REQUIRE(patch < subdivision);
        }
    }

    SECTION("a terrain that divides badly returns the first patch rather than a wild index")
    {
        // size/subdivision is integer division. A terrain whose size is smaller
        // than the subdivision divides to zero, and dividing by zero is what the
        // guard exists for.
        Terrain degenerate;
        degenerate.scale = 3 * 1 * 2;
        degenerate.size = 0;
        REQUIRE(degenerate.patchFor(1000.f) == 0);

        Terrain noScale;
        noScale.scale = 0;
        noScale.size = 256;
        REQUIRE(noScale.patchFor(1000.f) == 0);
    }
}

TEST_CASE("getHeight answers for inside the world and rejects outside it", "[terrain][query]")
{
    Terrain& terrain = loadedTerrain();

    SECTION("outside the terrain, there is no height to report")
    {
        // The guards are the contract here: anything a character or object might
        // reach that is beyond the terrain gets zero rather than a height read
        // from a tile that does not exist.
        REQUIRE(terrain.getHeight(-1.f, 50.f) == 0.f);
        REQUIRE(terrain.getHeight(50.f, -1.f) == 0.f);
        REQUIRE(terrain.getHeight(terrain.size * terrain.scale + 100, 50.f) == 0.f);
        REQUIRE(terrain.getHeight(50.f, terrain.size * terrain.scale + 100) == 0.f);
        REQUIRE(terrain.getHeight(0.f, 0.f) == 0.f);
    }

    SECTION("inside the terrain, the height is finite and repeatable")
    {
        for (float x = 20.f; x < terrain.size * terrain.scale; x += terrain.scale * 40) {
            for (float z = 20.f; z < terrain.size * terrain.scale; z += terrain.scale * 40) {
                INFO("at (" << x << ", " << z << ")");
                const float first = terrain.getHeight(x, z);
                const float second = terrain.getHeight(x, z);
                REQUIRE(std::isfinite(first));
                REQUIRE(first == second);

                // The heightmap is read from a 16-bit PNG and scaled, so the
                // world's vertical extent is bounded by the terrain scale. A height
                // outside that range means the interpolation ran away.
                REQUIRE(first > -terrain.scale * 2);
                REQUIRE(first < terrain.scale * (max_terrain_size + 1) * 2);
            }
        }
    }

    SECTION("adjacent samples do not jump by more than the terrain can slope")
    {
        // A sampling step of one tile cannot change the height by more than one
        // tile's worth of slope. If it does, the interpolation or the tile lookup
        // is wrong - and the vertical range above is loose enough not to catch it.
        const float step = terrain.scale;
        const float heightA = terrain.getHeight(wellInsideCoordinate(), wellInsideCoordinate());
        const float heightB = terrain.getHeight(wellInsideCoordinate() + step, wellInsideCoordinate());
        REQUIRE(std::fabs(heightB - heightA) <= terrain.scale);
    }
}

TEST_CASE("getHeightByTile reads the heightmap directly", "[terrain][query]")
{
    Terrain& terrain = loadedTerrain();

    SECTION("it matches the tile it names")
    {
        // The definitional relation: the tile one column over, times the scale.
        // Pinned because every other path through the heightmap either interpolates
        // over it or indexes it, and a drift between them is invisible otherwise.
        for (int x = 0; x < 20; x++) {
            for (int y = 0; y < 20; y++) {
                INFO("tile (" << x << ", " << y << ")");
                REQUIRE(terrain.getHeightByTile(x, y)
                        == terrain.heightmap->at(x + 1).at(y) * terrain.scale + .01f);
            }
        }
    }

    SECTION("a tile off the negative end is rejected by a guard, not a throw")
    {
        // Note the asymmetry, because it is easy to "fix" by mistake. The guards
        // on the negative side return zero rather than throwing, and x = -1 is not
        // guarded at all: it reads heightmap[0], which is a real tile. Only x of -2
        // or less, and any negative y, hit the guards.
        //
        // The positive side throws, because that is std::array refusing a read past
        // the end. The negative side returning zero is the historical contract and
        // callers depend on it.
        // Extra parentheses: Catch2 decomposes the expression and rejects a bare
        // "a == b || c == d" chain.
        const bool negativeOneIsZeroOrFirstTile = terrain.getHeightByTile(-1, 0) == 0.f
            || terrain.getHeightByTile(-1, 0) == terrain.heightmap->at(0).at(0) * terrain.scale + .01f;
        REQUIRE(negativeOneIsZeroOrFirstTile);
        REQUIRE(terrain.getHeightByTile(-2, 0) == 0.f);
        REQUIRE(terrain.getHeightByTile(-10, 5) == 0.f);
        REQUIRE(terrain.getHeightByTile(5, -1) == 0.f);
        REQUIRE(terrain.getHeightByTile(0, -5) == 0.f);
    }

    SECTION("a tile off the positive end is rejected")
    {
        // This guard was never there. heightmap is 257 wide and indexed at x+1,
        // so an x of 256 read one past the end - one of the out-of-bounds this
        // codebase had in the terrain family.
        REQUIRE_THROWS_AS(terrain.getHeightByTile(max_terrain_size, 0), std::out_of_range);
        REQUIRE_THROWS_AS(terrain.getHeightByTile(0, max_terrain_size + 1), std::out_of_range);
        REQUIRE_THROWS_AS(terrain.getHeightByTile(100000, 0), std::out_of_range);
    }
}

TEST_CASE("getOpacity answers within the unit range it blends from", "[terrain][query]")
{
    Terrain& terrain = loadedTerrain();

    SECTION("outside the terrain is fully transparent")
    {
        REQUIRE(terrain.getOpacity(-1.f, 50.f) == 0.f);
        REQUIRE(terrain.getOpacity(50.f, -1.f) == 0.f);
        REQUIRE(terrain.getOpacity(terrain.size * terrain.scale + 100, 50.f) == 0.f);
    }

    SECTION("inside the terrain the result is a blend of four tiles")
    {
        // If any one of the four surrounding tiles were read as garbage, this
        // would land outside the range the blend can produce from values that are
        // themselves inside it.
        for (float x = 20.f; x < terrain.size * terrain.scale; x += terrain.scale * 30) {
            for (float z = 20.f; z < terrain.size * terrain.scale; z += terrain.scale * 30) {
                INFO("at (" << x << ", " << z << ")");
                const float opacity = terrain.getOpacity(x, z);
                REQUIRE(std::isfinite(opacity));
                REQUIRE(opacity >= -0.001f);
                REQUIRE(opacity <= 1.001f);
            }
        }
    }
}

TEST_CASE("getNormal returns a unit vector pointing out of the surface", "[terrain][query]")
{
    Terrain& terrain = loadedTerrain();

    SECTION("outside the terrain the normal is zero, and the caller is told so")
    {
        // A zero vector from a "normal" is odd, but it is what the guards return,
        // and the drawing code relies on it being a normal it can normalise. It is
        // pinned so that a change here is visible rather than assumed.
        const Vector3 outsideNormal = terrain.getNormal(-1.f, 50.f);
        REQUIRE(outsideNormal.x == 0.f);
        REQUIRE(outsideNormal.y == 0.f);
        REQUIRE(outsideNormal.z == 0.f);
    }

    SECTION("inside the terrain the normal is unit length and faces upward")
    {
        const float extent = terrain.size * terrain.scale;
        for (float x = 20.f; x < extent; x += terrain.scale * 40) {
            for (float z = 20.f; z < extent; z += terrain.scale * 40) {
                INFO("at (" << x << ", " << z << ")");
                const Vector3 normal = terrain.getNormal(x, z);
                REQUIRE(std::isfinite(normal.x));
                REQUIRE(std::isfinite(normal.y));
                REQUIRE(std::isfinite(normal.z));

                const float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
                // Normalise is called before returning, so this is not optional.
                REQUIRE(std::fabs(length - 1.f) < 0.001f);

                // A ground surface points up. This is what everything that stands
                // on the terrain depends on.
                REQUIRE(normal.y > 0.f);
            }
        }
    }

    SECTION("the normal varies across the surface")
    {
        // Two far-apart samples should not be the same vector, otherwise the
        // bilinear blend would be reading one tile repeatedly.
        // Named to avoid 'near'/'far', which windows.h still defines as macros
        // from the 16-bit pointer era. 'far' silently becomes nothing and the
        // declaration becomes 'const Vector3 = ...'.
        const Vector3 closeUp = terrain.getNormal(wellInsideCoordinate(), wellInsideCoordinate());
        const Vector3 further = terrain.getNormal(wellInsideCoordinate() * 5, wellInsideCoordinate() * 8);
        const bool differs = std::fabs(closeUp.x - further.x) > 0.001f
            || std::fabs(closeUp.y - further.y) > 0.001f
            || std::fabs(closeUp.z - further.z) > 0.001f;
        REQUIRE(differs);
    }
}
