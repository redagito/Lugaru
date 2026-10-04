// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <SDL_scancode.h>

#include <type_traits>

#include "GameState.hpp"
#include "Tutorial.hpp"
#include "User/Settings.hpp"
#include "Utils/Input.hpp"

namespace
{

// Seeds for the twenty-eight GameState members DefaultSettings writes. Every
// member is seeded with the opposite of the value DefaultSettings pins, so that
// the assertions in the test below are all observations of a transition the
// function actually made. That matters most for the nine flags DefaultSettings
// turns off, because their pinned value is also the member's own starting
// value: without a seed, asserting on those would pass even if DefaultSettings
// had written nothing at all, or written to some other object entirely.
// `fullscreen` is the ninth of those, and is the sharpest case: its member
// default is false and DefaultSettings pins false, so only seeding it to true
// turns "assert it is false afterwards" into a real observation.
const float kSeedGameSpeed = 0.5f;
const unsigned short kSeedConsoleKey = 42;
const unsigned short kSeedCrouchKey = 43;
const unsigned short kSeedJumpKey = 52;
const unsigned short kSeedForwardKey = 45;
const unsigned short kSeedBackKey = 46;
const unsigned short kSeedLeftKey = 47;
const unsigned short kSeedRightKey = 48;
const unsigned short kSeedDrawKey = 49;
const unsigned short kSeedThrowKey = 50;
const unsigned short kSeedAttackKey = 51;
const float kSeedMouseSensitivity = 0.5f;
const float kSeedVolume = 0.25f;

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
	gamestate.ismotionblur = false;
	gamestate.usermousesensitivity = kSeedMouseSensitivity;
	gamestate.fullscreen = true;
	gamestate.musictoggle = false;
	gamestate.volume = kSeedVolume;
	gamestate.ambientsound = false;
	gamestate.crouchkey = kSeedCrouchKey;
	gamestate.jumpkey = kSeedJumpKey;
	gamestate.forwardkey = kSeedForwardKey;
	gamestate.backkey = kSeedBackKey;
	gamestate.leftkey = kSeedLeftKey;
	gamestate.rightkey = kSeedRightKey;
	gamestate.drawkey = kSeedDrawKey;
	gamestate.throwkey = kSeedThrowKey;
	gamestate.attackkey = kSeedAttackKey;
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

	// DefaultSettings writes twenty-eight GameState members, and every one of
	// them lands on the instance it was handed. If it regressed to writing a
	// hidden shared instance instead, all twenty-eight of these would still
	// hold their seed values and fail here.
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
	REQUIRE(injected.ismotionblur == true);
	REQUIRE(injected.usermousesensitivity == 1.0f);
	REQUIRE(injected.fullscreen == false);
	REQUIRE(injected.musictoggle == true);
	REQUIRE(injected.volume == 0.8f);
	REQUIRE(injected.ambientsound == true);
	REQUIRE(injected.crouchkey == SDL_SCANCODE_LSHIFT);
	REQUIRE(injected.jumpkey == SDL_SCANCODE_SPACE);
	REQUIRE(injected.forwardkey == SDL_SCANCODE_W);
	REQUIRE(injected.backkey == SDL_SCANCODE_S);
	REQUIRE(injected.leftkey == SDL_SCANCODE_A);
	REQUIRE(injected.rightkey == SDL_SCANCODE_D);
	REQUIRE(injected.drawkey == SDL_SCANCODE_E);
	REQUIRE(injected.throwkey == SDL_SCANCODE_Q);
	REQUIRE(injected.attackkey == MOUSEBUTTON_LEFT);

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
	REQUIRE(bystander.ismotionblur == false);
	REQUIRE(bystander.usermousesensitivity == kSeedMouseSensitivity);
	REQUIRE(bystander.fullscreen == true);
	REQUIRE(bystander.musictoggle == false);
	REQUIRE(bystander.volume == kSeedVolume);
	REQUIRE(bystander.ambientsound == false);
	REQUIRE(bystander.crouchkey == kSeedCrouchKey);
	REQUIRE(bystander.jumpkey == kSeedJumpKey);
	REQUIRE(bystander.forwardkey == kSeedForwardKey);
	REQUIRE(bystander.backkey == kSeedBackKey);
	REQUIRE(bystander.leftkey == kSeedLeftKey);
	REQUIRE(bystander.rightkey == kSeedRightKey);
	REQUIRE(bystander.drawkey == kSeedDrawKey);
	REQUIRE(bystander.throwkey == kSeedThrowKey);
	REQUIRE(bystander.attackkey == kSeedAttackKey);

	SECTION("driving a different instance leaves the first alone")
	{
		// Re-seed the first instance, because the body above has already driven
		// it to the pinned values; without a fresh seed, "unchanged" would not
		// tell "untouched" apart from "written again to the same value".
		seedUnpinnedValues(injected);

		DefaultSettings(bystander);

		// The call landed on all twenty-eight members of the instance it was given.
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
		REQUIRE(bystander.ismotionblur == true);
		REQUIRE(bystander.usermousesensitivity == 1.0f);
		REQUIRE(bystander.fullscreen == false);
		REQUIRE(bystander.musictoggle == true);
		REQUIRE(bystander.volume == 0.8f);
		REQUIRE(bystander.ambientsound == true);
		REQUIRE(bystander.crouchkey == SDL_SCANCODE_LSHIFT);
		REQUIRE(bystander.jumpkey == SDL_SCANCODE_SPACE);
		REQUIRE(bystander.forwardkey == SDL_SCANCODE_W);
		REQUIRE(bystander.backkey == SDL_SCANCODE_S);
		REQUIRE(bystander.leftkey == SDL_SCANCODE_A);
		REQUIRE(bystander.rightkey == SDL_SCANCODE_D);
		REQUIRE(bystander.drawkey == SDL_SCANCODE_E);
		REQUIRE(bystander.throwkey == SDL_SCANCODE_Q);
		REQUIRE(bystander.attackkey == MOUSEBUTTON_LEFT);

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
		REQUIRE(injected.ismotionblur == false);
		REQUIRE(injected.usermousesensitivity == kSeedMouseSensitivity);
		REQUIRE(injected.fullscreen == true);
		REQUIRE(injected.musictoggle == false);
		REQUIRE(injected.volume == kSeedVolume);
		REQUIRE(injected.ambientsound == false);
		REQUIRE(injected.crouchkey == kSeedCrouchKey);
		REQUIRE(injected.jumpkey == kSeedJumpKey);
		REQUIRE(injected.forwardkey == kSeedForwardKey);
		REQUIRE(injected.backkey == kSeedBackKey);
		REQUIRE(injected.leftkey == kSeedLeftKey);
		REQUIRE(injected.rightkey == kSeedRightKey);
		REQUIRE(injected.drawkey == kSeedDrawKey);
		REQUIRE(injected.throwkey == kSeedThrowKey);
		REQUIRE(injected.attackkey == kSeedAttackKey);
	}
}
