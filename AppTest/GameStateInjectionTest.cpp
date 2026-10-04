// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <SDL_scancode.h>

#include <type_traits>

#include "GameState.hpp"
#include "Tutorial.hpp"
#include "User/Settings.hpp"

namespace
{

// Seeds for the thirteen GameState members DefaultSettings writes. Every member
// is seeded with the opposite of the value DefaultSettings pins, so that the
// assertions in the test below are all observations of a transition the function
// actually made. That matters most for the eight flags DefaultSettings turns
// off, because their pinned value is also the member's own starting value:
// without a seed, asserting on those would pass even if DefaultSettings had
// written nothing at all, or written to some other object entirely.
const float kSeedGameSpeed = 0.5f;
const unsigned short kSeedConsoleKey = 42;

void seedUnpinnedValues(GameState& gamestate)
{
	gamestate.floatjump = true;
	gamestate.autoslomo = false;
	gamestate.invertmouse = true;
	gamestate.foliage = false;
	gamestate.gamespeed = kSeedGameSpeed;
	gamestate.damageeffects = true;
	gamestate.texttoggle = false;
	gamestate.alwaysblur = true;
	gamestate.showpoints = true;
	gamestate.showdamagebar = true;
	gamestate.immediate = true;
	gamestate.velocityblur = true;
	gamestate.consolekey = kSeedConsoleKey;
}

} // namespace

TEST_CASE("functions that mutate game state take the GameState to mutate", "[gamestate][injectability]")
{
	// Red while GameState is only reachable through a process-wide accessor:
	// none of these entry points can be handed an instance, so a caller cannot
	// own the object the engine writes to.
	SECTION("the settings entry points accept an injected GameState")
	{
		REQUIRE(std::is_invocable_v<decltype(&DefaultSettings), GameState&>);
		REQUIRE(std::is_invocable_v<decltype(&SaveSettings), GameState&>);
		REQUIRE(std::is_invocable_v<decltype(&LoadSettings), GameState&>);
	}

	SECTION("a function that never writes GameState does not take one")
	{
		// Negative control. `DefaultSettings` used to take no arguments at all, so
		// requiring that it is NOT callable with zero arguments proves the trait
		// really distinguishes the injected shape from the old global-accessor
		// shape, rather than holding for any one-argument signature.
		REQUIRE_FALSE(std::is_invocable_v<decltype(&DefaultSettings)>);
		REQUIRE_FALSE(std::is_invocable_v<decltype(&SaveSettings)>);
	}
}

TEST_CASE("an injected GameState is the only instance a function writes to", "[gamestate][injectability]")
{
	GameState injected;
	GameState bystander;

	// Both instances start from seeds DefaultSettings would never leave behind,
	// so every assertion below reports a transition the function actually made.
	seedUnpinnedValues(injected);
	seedUnpinnedValues(bystander);

	DefaultSettings(injected);

	// DefaultSettings writes thirteen GameState members, and every one of them
	// lands on the instance it was handed. If it regressed to writing a hidden
	// shared instance instead, all thirteen of these would still hold their seed
	// values and fail here.
	REQUIRE(injected.floatjump == false);
	REQUIRE(injected.autoslomo == true);
	REQUIRE(injected.invertmouse == false);
	REQUIRE(injected.foliage == true);
	REQUIRE(injected.gamespeed == 1.0f);
	REQUIRE(injected.damageeffects == false);
	REQUIRE(injected.texttoggle == true);
	REQUIRE(injected.alwaysblur == false);
	REQUIRE(injected.showpoints == false);
	REQUIRE(injected.showdamagebar == false);
	REQUIRE(injected.immediate == false);
	REQUIRE(injected.velocityblur == false);
	REQUIRE(injected.consolekey == SDL_SCANCODE_GRAVE);

	// The point of injecting the object: a second, independently constructed
	// GameState keeps every value it was seeded with, so nothing outside the
	// instance that was passed in was written.
	REQUIRE(bystander.floatjump == true);
	REQUIRE(bystander.autoslomo == false);
	REQUIRE(bystander.invertmouse == true);
	REQUIRE(bystander.foliage == false);
	REQUIRE(bystander.gamespeed == kSeedGameSpeed);
	REQUIRE(bystander.damageeffects == true);
	REQUIRE(bystander.texttoggle == false);
	REQUIRE(bystander.alwaysblur == true);
	REQUIRE(bystander.showpoints == true);
	REQUIRE(bystander.showdamagebar == true);
	REQUIRE(bystander.immediate == true);
	REQUIRE(bystander.velocityblur == true);
	REQUIRE(bystander.consolekey == kSeedConsoleKey);

	SECTION("driving a different instance leaves the first alone")
	{
		// Re-seed the first instance, because the body above has already driven
		// it to the pinned values; without a fresh seed, "unchanged" would not
		// tell "untouched" apart from "written again to the same value".
		seedUnpinnedValues(injected);

		DefaultSettings(bystander);

		// The call landed on all thirteen members of the instance it was given.
		REQUIRE(bystander.floatjump == false);
		REQUIRE(bystander.autoslomo == true);
		REQUIRE(bystander.invertmouse == false);
		REQUIRE(bystander.foliage == true);
		REQUIRE(bystander.gamespeed == 1.0f);
		REQUIRE(bystander.damageeffects == false);
		REQUIRE(bystander.texttoggle == true);
		REQUIRE(bystander.alwaysblur == false);
		REQUIRE(bystander.showpoints == false);
		REQUIRE(bystander.showdamagebar == false);
		REQUIRE(bystander.immediate == false);
		REQUIRE(bystander.velocityblur == false);
		REQUIRE(bystander.consolekey == SDL_SCANCODE_GRAVE);

		// ...and the instance that was not passed in kept every seeded value.
		REQUIRE(injected.floatjump == true);
		REQUIRE(injected.autoslomo == false);
		REQUIRE(injected.invertmouse == true);
		REQUIRE(injected.foliage == false);
		REQUIRE(injected.gamespeed == kSeedGameSpeed);
		REQUIRE(injected.damageeffects == true);
		REQUIRE(injected.texttoggle == false);
		REQUIRE(injected.alwaysblur == true);
		REQUIRE(injected.showpoints == true);
		REQUIRE(injected.showdamagebar == true);
		REQUIRE(injected.immediate == true);
		REQUIRE(injected.velocityblur == true);
		REQUIRE(injected.consolekey == kSeedConsoleKey);
	}
}
