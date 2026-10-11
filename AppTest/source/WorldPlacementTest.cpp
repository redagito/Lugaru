// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// A regression test for a specific, serious bug. While instrumenting the level
// load with timers, an edit dropped the call that puts the level's objects onto
// the terrain's patch table. Everything still compiled, every other test still
// passed, and the level was still fully loaded - but the patch table was empty,
// so nothing ever collided with anything and the player walked through solid
// geometry.
//
// That is invisible to a compile check and invisible to "does the level load",
// which is exactly why it deserves a test of its own: the world has to be
// *placed on* the terrain, not merely loaded.

#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "Console.hpp"
#include "Environment/Terrain.hpp"
#include "Game.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Objects/Object.hpp"
#include "Objects/Person.hpp"
#include "Objects/PersonType.hpp"
#include "Utils/Folders.hpp"

namespace
{

// Every collidable object the loader built, which is every object except the
// three kinds addToTerrain deliberately leaves off the table: tree leaves,
// bushes and fires.
int objectsThatShouldBeOnTheTerrain()
{
	int count = 0;
	for (unsigned i = 0; i < Object::objects.size(); i++) {
		const object_type type = Object::objects[i]->type;
		if (type != treeleavestype && type != bushtype && type != firetype) {
			count++;
		}
	}
	return count;
}

int totalPatchEntries(const Terrain& terrain)
{
	int total = 0;
	for (int i = 0; i < subdivision; i++) {
		for (int j = 0; j < subdivision; j++) {
			total += static_cast<int>(terrain.patchobjects[i][j].size());
		}
	}
	return total;
}

} // namespace

TEST_CASE("a loaded level's objects are placed on the terrain", "[levelload][collision]")
{
	static GameState gamestate;
	static GameAssets assets;
	static Console console;
	static bool typesLoaded = false;
	if (!typesLoaded) {
		PersonType::Load(assets.graphics);
		typesLoaded = true;
	}
	if (!assets.terrain) {
		assets.terrain = std::make_unique<Terrain>();
	}
	// LoadStuff sets the world scale before any level loads. Without it the
	// terrain would be 256 units across while the shipped levels place their
	// geometry out past 700, so every object would fall outside every patch.
	assets.terrain->scale = 3 * 1 * 2;

	gamestate.visibleloading = false;
	gamestate.gameon = 1;
	gamestate.stillloading = 0;

	REQUIRE(Game::LoadJsonLevel("Lugaru1", false, gamestate, assets, console));

	REQUIRE(Object::objects.size() > 0);
	REQUIRE(Person::players.size() > 0);

	// The patch table must actually be populated. It being empty is the bug
	// this test exists for: loading succeeds either way.
	REQUIRE(totalPatchEntries(*assets.terrain) > 0);

	// And every collidable object must be reachable through it. An object that
	// loaded but was never registered is one the player cannot hit.
	for (unsigned i = 0; i < Object::objects.size(); i++) {
		const object_type type = Object::objects[i]->type;
		if (type == treeleavestype || type == bushtype || type == firetype) {
			continue;
		}
		const Vector3& position = Object::objects[i]->position;
		const int patchx = assets.terrain->patchFor(position.x);
		const int patchz = assets.terrain->patchFor(position.z);
		INFO("object " << i << " at " << position.x << "," << position.z << " patch " << patchx << "," << patchz);
		bool registered = false;
		for (unsigned k = 0; k < assets.terrain->patchobjects[patchx][patchz].size(); k++) {
			if (assets.terrain->patchobjects[patchx][patchz][k] == i) {
				registered = true;
			}
		}
		REQUIRE(registered);
	}

	// The player's own spawn must be surrounded by registered geometry. This is
	// the case that was visibly broken: the rock beside the spawn point.
	const Vector3 spawn = Person::players[0]->coords;
	INFO("player spawn " << spawn.x << "," << spawn.z);
	const int playerPatchX = assets.terrain->patchFor(spawn.x);
	const int playerPatchZ = assets.terrain->patchFor(spawn.z);
	REQUIRE(assets.terrain->patchobjects[playerPatchX][playerPatchZ].size() > 0);

	// And the query the collision code performs must be able to find something.
	// Queried from the player's spawn it may legitimately find nothing - the
	// spawn point is not necessarily touching geometry - so query from the
	// position of the first registered object, which cannot miss itself.
	REQUIRE(!Object::objects.empty());
	for (unsigned i = 0; i < Object::objects.size(); i++) {
		Object::objects[i]->possible = false;
	}
	Vector3 atObject = Object::objects[0]->position;
	Object::SphereCheckPossible(&atObject, 3, *assets.terrain);
	REQUIRE(Object::objects[0]->possible);
}
