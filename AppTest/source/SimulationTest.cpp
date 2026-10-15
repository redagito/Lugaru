// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// Characterisation tests for the simulation entry points. Their job is to pin
// current behaviour so that splitting the god-functions can be shown not to
// change it: Person::doAI is ~1100 lines, Person::RagDoll ~170, and both are
// named in the roadmap for decomposition. Without these, a split that moved a
// line across the wrong boundary would still compile and still pass every other
// test, and the difference would only show up in a playtest.
//
// What is asserted is invariants, not values. "Joints stay finite", "the body
// does not leave the world", "nothing diverges" - the things that must hold for
// every level and every frame. Exact positions are deliberately not pinned: they
// change whenever anything legitimately changes, and a test that has to be
// updated for every real edit stops being read.
//
// doAI and RagDoll are called directly rather than through Game::Tick because
// they contain no GL calls, so they run in a build with no context. Tick itself
// reaches DrawGLScene's neighbours and so needs the stub library.

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
#include "Animation/Animation.hpp"
#include "Utils/Folders.hpp"

namespace
{

bool everyJointIsFinite(const Person& person)
{
	for (const Joint& joint : person.skeleton.joints) {
		if (!std::isfinite(joint.position.x) || !std::isfinite(joint.position.y) || !std::isfinite(joint.position.z)) {
			return false;
		}
		if (!std::isfinite(joint.velocity.x) || !std::isfinite(joint.velocity.y) || !std::isfinite(joint.velocity.z)) {
			return false;
		}
	}
	return true;
}

bool isWithinReachOfTheWorld(const Person& person, const Terrain& terrain)
{
	// Terrain coordinates run from 0 to size * scale. A body that has left that
	// box has escaped, which is what a broken collision or an unclamped
	// integration step looks like.
	const float limit = terrain.size * terrain.scale + 200;
	return person.coords.x > -limit && person.coords.x < limit
		&& person.coords.y > -limit && person.coords.y < limit
		&& person.coords.z > -limit && person.coords.z < limit;
}

} // namespace

TEST_CASE("the simulation runs a loaded level without diverging", "[simulation]")
{
	static GameState gamestate;
	static GameAssets assets;
	static Console console;
	static bool typesLoaded = false;
	if (!typesLoaded) {
		PersonType::Load(assets.graphics);
        // Person::doAI subscripts assets.graphics.animations, which the level
        // load leaves empty. Without this the STL's checked subscript fast-fails.
        Animation::loadAll([]() {}, assets.graphics);
		typesLoaded = true;
	}
	if (!assets.terrain) {
		assets.terrain = std::make_unique<Terrain>();
	}
	assets.terrain->scale = 3 * 1 * 2;

	gamestate.visibleloading = false;
	gamestate.gameon = 1;
	gamestate.stillloading = 0;

	REQUIRE(Game::LoadJsonLevel("Lugaru1", false, gamestate, assets, console));
	REQUIRE(Person::players.size() > 0);
	REQUIRE(assets.terrain->size > 0);

	// Not const: the simulation writes to the terrain. RagDoll lays decals and
	// DoConstraints moves joints against it, so a const view would be a lie.
	Terrain& terrain = *assets.terrain;
	// The engine has a start array but no end array; GameState carries a
	// whichjointendarray that nothing in these paths reads.
	int jointStart[26] = {};

	SECTION("the AI can be run for many frames on every character")
	{
		for (int frame = 0; frame < 120; frame++) {
			for (const std::shared_ptr<Person>& player : Person::players) {
				player->doAI(terrain, false, false, .01f, gamestate, assets);
			}
		}

		for (const std::shared_ptr<Person>& player : Person::players) {
			INFO("player " << player.get() << " after 120 frames of AI");
			REQUIRE(everyJointIsFinite(*player));
			REQUIRE(isWithinReachOfTheWorld(*player, terrain));
		}
	}

	SECTION("a ragdoll can be simulated for many frames")
	{
		// Ragdolling a body is the state a killed character ends up in, and it is
		// the part of the simulation that integrates joints against the terrain
		// and the objects on it - the same path the missing AddObjectsToTerrain
		// call once silently broke.
		for (unsigned i = 0; i < Person::players.size(); i++) {
			Person::players[i]->skeleton.free = 1;
		}

		for (int frame = 0; frame < 240; frame++) {
			for (const std::shared_ptr<Person>& player : Person::players) {
				player->RagDoll(true, terrain, false, false, .01f, jointStart, gamestate, assets);
			}
		}

		for (const std::shared_ptr<Person>& player : Person::players) {
			INFO("player " << player.get() << " after 240 frames of ragdoll");
			REQUIRE(everyJointIsFinite(*player));
			REQUIRE(isWithinReachOfTheWorld(*player, terrain));
		}
	}

	SECTION("damage accumulates and never produces an impossible state")
	{
		for (const std::shared_ptr<Person>& player : Person::players) {
			for (int hit = 0; hit < 40; hit++) {
				player->DoDamage(5, terrain, false, false, .01f, jointStart, gamestate, assets);
			}
			INFO("player " << player.get() << " after 40 hits");
			REQUIRE(std::isfinite(player->damage));
			REQUIRE(std::isfinite(player->permanentdamage));
			REQUIRE(std::isfinite(player->bloodloss));
			// Blood loss is only ever added to, so it cannot go negative, and
			// permanent damage cannot exceed the sum of what was dealt.
			REQUIRE(player->bloodloss >= 0);
			REQUIRE(player->permanentdamage >= 0);
			REQUIRE(player->damage >= 0);
		}
	}
}
