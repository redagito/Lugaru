// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "GameState.hpp"
#include "Tutorial.hpp"
#include "User/Settings.hpp"

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
		// Negative control. Without it the checks above could hold for any
		// signature at all; this pins that the trait really distinguishes the
		// two shapes.
		REQUIRE_FALSE(std::is_invocable_v<decltype(&Tutorial::DoStuff), GameState&>);
	}
}

TEST_CASE("an injected GameState is the only instance a function writes to", "[gamestate][injectability]")
{
	GameState injected;
	GameState bystander;

	DefaultSettings(injected);

	// DefaultSettings pins the time scale, the one GameState member it touches.
	REQUIRE(injected.gamespeed == 1.0f);

	// The point of injecting the object: a second, independently constructed
	// GameState is untouched, so no hidden shared instance was written instead.
	REQUIRE(bystander.gamespeed == 0.0f);

	SECTION("driving a different instance leaves the first alone")
	{
		bystander.gamespeed = 0.25f;
		DefaultSettings(bystander);
		REQUIRE(bystander.gamespeed == 1.0f);
		REQUIRE(injected.gamespeed == 1.0f);
	}
}