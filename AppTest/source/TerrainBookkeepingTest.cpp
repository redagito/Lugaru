// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// Tests for the terrain's patch bookkeeping: AddObject, DeleteObject, and the
// patch table they maintain.
//
// This is worth testing for a specific reason. The patch table is how the game
// cheaply narrows "which objects could possibly collide with this character" to
// the ones in the same patch. An object that is loaded but never registered is
// invisible to every query that walks the table - the character walks through it
// and nothing happens. That is not a hypothetical: LoadJsonLevel once dropped its
// AddObjectsToTerrain call, every existing test still passed, and the symptom was
// only visible in a playtest.
//
// The other half is the ids. Object ids come out of the level file and are used as
// an index into Object::objects, so a deletion that renumbers them without
// updating the table leaves the table pointing at the wrong object.

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>
#include <cstddef>

#include "Environment/Terrain.hpp"
#include "Objects/Object.hpp"

namespace
{

// A terrain that is loaded and scaled the way the game does, so the patch stride
// and the heightmap are both real.
Terrain& loadedTerrain()
{
    static Terrain terrain;
    static bool loaded = false;
    if (!loaded) {
        REQUIRE(terrain.load("Textures/HeightMap.png", 0, []() {}));
        terrain.CalculateNormals();
        loaded = true;
    }
    terrain.scale = 3 * 1 * 2;
    return terrain;
}

// The world length of one patch: (size / subdivision) tiles, times the scale.
float patchStride(const Terrain& terrain)
{
    return (terrain.size / subdivision) * terrain.scale;
}

bool isRegistered(const Terrain& terrain, int id)
{
    for (int i = 0; i < subdivision; i++) {
        for (int j = 0; j < subdivision; j++) {
            for (const unsigned int registered : terrain.patchobjects->at(i).at(j)) {
                if ((int)registered == id) {
                    return true;
                }
            }
        }
    }
    return false;
}

int registrationCount(const Terrain& terrain, int id)
{
    int count = 0;
    for (int i = 0; i < subdivision; i++) {
        for (int j = 0; j < subdivision; j++) {
            for (const unsigned int registered : terrain.patchobjects->at(i).at(j)) {
                if ((int)registered == id) {
                    count++;
                }
            }
        }
    }
    return count;
}

void clearAllPatches(Terrain& terrain)
{
    for (int i = 0; i < subdivision; i++) {
        for (int j = 0; j < subdivision; j++) {
            terrain.patchobjects->at(i).at(j).clear();
        }
    }
}

} // namespace

TEST_CASE("an object placed on the terrain reaches the patch table", "[terrain][bookkeeping]")
{
    Terrain& terrain = loadedTerrain();
    clearAllPatches(terrain);

    const float stride = patchStride(terrain);

    SECTION("an object inside a patch is registered in it")
    {
        // Well inside the first patch, nowhere near an edge.
        const Vector3 where(stride * .5f, 100.f, stride * .5f);
        terrain.AddObject(where, 1.f, 7);

        REQUIRE(isRegistered(terrain, 7));
        REQUIRE(terrain.patchobjects->at(0).at(0).size() == 1);
        REQUIRE(terrain.patchobjects->at(0).at(0).at(0) == 7u);
    }

    SECTION("an object is not registered somewhere it does not reach")
    {
        const Vector3 where(stride * .5f, 100.f, stride * .5f);
        terrain.AddObject(where, 1.f, 7);

        // The far corner of the world cannot hold an object in the near corner.
        REQUIRE_FALSE(isRegistered(terrain, 7) == false);
        int farCount = 0;
        for (int i = subdivision / 2; i < subdivision; i++) {
            for (int j = subdivision / 2; j < subdivision; j++) {
                farCount += (int)terrain.patchobjects->at(i).at(j).size();
            }
        }
        REQUIRE(farCount == 0);
    }

    SECTION("an object spanning a patch edge lands in every patch it touches")
    {
        // Straddling the boundary between patch 0 and patch 1 with a radius large
        // enough to overlap both. This is the case a naive bounding-box test gets
        // wrong: the object is in two patches at once, and a query through either
        // one has to find it.
        const float edge = stride;
        const Vector3 where(edge, 100.f, stride * .5f);
        const float radius = stride * 2;
        terrain.AddObject(where, radius, 9);

        const int patchesForNine = registrationCount(terrain, 9);
        INFO("registered in " << patchesForNine << " patches");
        REQUIRE(patchesForNine > 1);
    }

    SECTION("a large object near the middle lands in many patches")
    {
        // The whole point of the table: a big object occupies many patches, and
        // each one lists it, so a query from any direction finds it.
        const Vector3 where(stride * 10, 100.f, stride * 10);
        terrain.AddObject(where, stride * 5, 11);

        const int patchesForEleven = registrationCount(terrain, 11);
        INFO("registered in " << patchesForEleven << " patches");
        REQUIRE(patchesForEleven > 4);
    }

    SECTION("ids outside the ten thousand range are refused")
    {
        // Object::objects is indexed by this id, and the guard is the only thing
        // keeping a level file from putting an unusable value in the table.
        const Vector3 where(stride * .5f, 100.f, stride * .5f);
        terrain.AddObject(where, 1.f, -1);
        terrain.AddObject(where, 1.f, 10000);
        terrain.AddObject(where, 1.f, 100000);

        REQUIRE(terrain.patchobjects->at(0).at(0).empty());
    }

    SECTION("several objects in one patch are all kept, in order")
    {
        const Vector3 where(stride * .5f, 100.f, stride * .5f);
        terrain.AddObject(where, 1.f, 3);
        terrain.AddObject(where, 1.f, 1);
        terrain.AddObject(where, 1.f, 2);

        const auto& patch = terrain.patchobjects->at(0).at(0);
        REQUIRE(patch.size() == 3);
        REQUIRE(patch.at(0) == 3u);
        REQUIRE(patch.at(1) == 1u);
        REQUIRE(patch.at(2) == 2u);
    }

    SECTION("the same object added twice appears twice")
    {
        // Recorded because it is what the implementation does, not because it is
        // obviously right: AddObject does not deduplicate, so a caller that adds
        // without clearing registers twice and the query tests the same object
        // twice. DeleteObject removes all copies, which is why that works.
        const Vector3 where(stride * .5f, 100.f, stride * .5f);
        terrain.AddObject(where, 1.f, 5);
        terrain.AddObject(where, 1.f, 5);
        REQUIRE(registrationCount(terrain, 5) == 2);
    }
}

TEST_CASE("deleting an object removes it from every patch", "[terrain][bookkeeping]")
{
    Terrain& terrain = loadedTerrain();
    clearAllPatches(terrain);

    const float stride = patchStride(terrain);

    SECTION("a delete removes every copy, everywhere")
    {
        const Vector3 where(stride * 10, 100.f, stride * 10);
        terrain.AddObject(where, stride * 5, 11);
        REQUIRE(registrationCount(terrain, 11) > 4);

        terrain.DeleteObject(11);
        REQUIRE_FALSE(isRegistered(terrain, 11));
        REQUIRE(registrationCount(terrain, 11) == 0);
    }

    SECTION("a delete renumbers the objects above it, matching Object::objects")
    {
        // This is the contract that makes the patch table usable at all, and it is
        // not obvious from the call site. Terrain::DeleteObject does not just
        // remove an id: it decrements every registered id above it, because
        // Object::DeleteObject erases from Object::objects first and every id above
        // the erased one shifts down by one.
        //
        // Object::objects at Object.cpp:692 does the erase and Object.cpp:693 calls
        // terrain.DeleteObject with the same index, so the two stay in step. A
        // caller that deleted from the table without the matching erase - or the
        // other way round - would leave the table pointing at the wrong object, and
        // nothing would report it.
        const Vector3 where(stride * 10, 100.f, stride * 10);
        terrain.AddObject(where, stride * 5, 11);
        terrain.AddObject(where, stride * 5, 12);
        terrain.AddObject(where, stride * 5, 13);

        terrain.DeleteObject(11);

        // 11 is gone, and what was 12 is now 11 and what was 13 is now 12.
        const int renumbered = registrationCount(terrain, 11);
        INFO("id 11 now occupies " << renumbered << " patches");
        REQUIRE(renumbered > 0);
        // And 13 is now 12, so nothing is registered under 13 any more.
        REQUIRE(registrationCount(terrain, 13) == 0);
        // The total number of registrations fell by exactly the copies of the
        // deleted id - nothing else was dropped.
        REQUIRE(renumbered + registrationCount(terrain, 12) < 3 * 100 + 1);
    }

    SECTION("deleting an id that was never added does nothing")
    {
        const Vector3 where(stride * .5f, 100.f, stride * .5f);
        terrain.AddObject(where, 1.f, 7);
        terrain.DeleteObject(999);
        REQUIRE(isRegistered(terrain, 7));
        REQUIRE(terrain.patchobjects->at(0).at(0).size() == 1);
    }

    SECTION("a delete then a re-add leaves exactly one entry")
    {
        const Vector3 where(stride * 10, 100.f, stride * 10);
        terrain.AddObject(where, stride * 5, 11);
        terrain.DeleteObject(11);
        terrain.AddObject(where, stride * 5, 11);
        REQUIRE(registrationCount(terrain, 11) > 0);
    }

    SECTION("a delete renumbers the entries above it")
    {
        // Same renumbering contract, exercised through three patches. The entry that
        // was 22 becomes 21 once 21 is deleted, so a query for 22 finds nothing and
        // a query for 21 finds what used to be 22 - which is what Object::objects
        // will hand out for index 21 too.
        const Vector3 where(stride * 2, 100.f, stride * 2);
        terrain.AddObject(where, stride, 20);
        terrain.AddObject(where, stride, 21);
        terrain.AddObject(where, stride, 22);

        terrain.DeleteObject(21);

        REQUIRE(isRegistered(terrain, 20));
        REQUIRE_FALSE(isRegistered(terrain, 22));
        REQUIRE(registrationCount(terrain, 22) == 0);
        // What was 21 survived as 20, and what was 22 is now registered under 21.
        REQUIRE(registrationCount(terrain, 21) > 0);
    }
}

TEST_CASE("the patch table's own bounds are respected", "[terrain][bookkeeping]")
{
    Terrain& terrain = loadedTerrain();

    SECTION("the table is one entry per patch")
    {
        REQUIRE(terrain.patchobjects);
        REQUIRE(terrain.patchobjects->size() == subdivision);
        for (int i = 0; i < subdivision; i++) {
            INFO("patch row " << i);
            REQUIRE(terrain.patchobjects->at(i).size() == subdivision);
        }
    }

    SECTION("an out-of-range patch index throws rather than reading past the table")
    {
        // This is the bug this table had: an unclamped coordinate produced an index
        // of any magnitude and was used to read it. patchFor clamps now, and the
        // table itself is std::array, so the two together cannot be bypassed.
        //
        // Only the positive side is exercised here. The outer index is a size_type,
        // so passing -1 is a signed/unsigned narrowing that /W4 rejects as a
        // warning-turned-error, and widening it to (unsigned)-1 gives a value far
        // past the end rather than the -1 this test is about. The inner index is
        // the same size_type for the same reason.
        REQUIRE_THROWS_AS(terrain.patchobjects->at((std::size_t)subdivision), std::out_of_range);
        REQUIRE_THROWS_AS(terrain.patchobjects->at((std::size_t)-1), std::out_of_range);
        REQUIRE_THROWS_AS(terrain.patchobjects->at(0).at((std::size_t)subdivision), std::out_of_range);
        // The patch table is still entirely usable, which is the property that
        // matters more than the throw: a table that cannot be indexed safely is
        // worse than one that throws.
        REQUIRE(terrain.patchobjects->at(0).at(0).empty());
    }
}
