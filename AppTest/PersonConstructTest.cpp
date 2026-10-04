// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "GameState.hpp"
#include "Objects/Person.hpp"
#include "Objects/PersonType.hpp"

namespace
{

// Person's constructor reads PersonType::types[creature], so the table must be
// populated before any Person can be built.
void ensurePersonTypesLoaded()
{
	static bool loaded = false;
	if (!loaded) {
		PersonType::Load();
		loaded = true;
	}
}

std::shared_ptr<Person> makePerson()
{
	ensurePersonTypesLoaded();
	// Person only needs the GameState for its loading-progress callback, so a
	// throwaway instance is enough here.
	GameState gamestate;
	return std::make_shared<Person>(gamestate);
}

} // namespace

TEST_CASE("a Person can be constructed in a unit test", "[person]")
{
	SECTION("defaults are sane once the type table is loaded")
	{
		const auto person = makePerson();
		REQUIRE(person->creature == rabbittype);
		REQUIRE(person->damagetolerance == 200);
		REQUIRE(person->num_weapons == 0);
		REQUIRE(person->numwaypoints == 0);
	}

	SECTION("each instance owns its own state")
	{
		auto a = makePerson();
		auto b = makePerson();
		a->num_weapons = 3;
		REQUIRE(b->num_weapons == 0);
	}
}

TEST_CASE("a running character turns toward its target yaw", "[person][aerial]")
{
	// Regression test: the running branch compared `targetyaw` (a float angle)
	// against animation_type enumerators instead of `animTarget`, so the
	// "not currently in a run animation" test never actually excluded anything
	// and the clause was true for essentially every yaw value.
	auto person = makePerson();
	person->targetyaw = 90.0f;

	SECTION("an idle character does not steer from this path")
	{
		// isRun() is false for the idle animation, and idle is not in the list of
		// animations that force steering, so this predicate is false.
		person->animTarget = bounceidleanim;
		REQUIRE_FALSE(person->shouldTurnTowardTarget());
	}

	SECTION("steers while walking")
	{
		person->animTarget = walkanim;
		REQUIRE(person->shouldTurnTowardTarget());
	}

	SECTION("does not steer while in a running animation")
	{
		person->animTarget = rabbitrunninganim;
		person->frameTarget = 0;
		REQUIRE_FALSE(person->shouldTurnTowardTarget());
	}

	SECTION("does not steer while in a wolf running animation")
	{
		person->animTarget = wolfrunninganim;
		person->frameTarget = 0;
		REQUIRE_FALSE(person->shouldTurnTowardTarget());
	}

	SECTION("a running animation at frame 4 still steers")
	{
		person->animTarget = rabbitrunninganim;
		person->frameTarget = 4;
		REQUIRE(person->shouldTurnTowardTarget());
	}

	SECTION("steering does not depend on the target yaw value")
	{
		// The old comparison was targetyaw (a float) against animation_type ids, so
		// a yaw numerically equal to one of those ids spuriously suppressed steering.
		person->animTarget = walkanim;
		for (float yaw : { 0.0f, 42.0f, 43.0f, 137.0f, -90.0f }) {
			person->targetyaw = yaw;
			REQUIRE(person->shouldTurnTowardTarget());
		}
	}

	SECTION("a yaw matching an animation id no longer suppresses steering")
	{
		person->animTarget = walkanim;
		person->targetyaw = static_cast<float>(rabbitrunninganim);
		REQUIRE(person->shouldTurnTowardTarget());
	}

	SECTION("other animations that force steering")
	{
		for (animation_type anim : { removeknifeanim, crouchremoveknifeanim, flipanim, fightsidestep, walkanim }) {
			person->animTarget = anim;
			REQUIRE(person->shouldTurnTowardTarget());
		}
	}
}