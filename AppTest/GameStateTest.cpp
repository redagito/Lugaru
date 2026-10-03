// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "GameState.hpp"

TEST_CASE("GameState members start at their historical global defaults", "[gamestate]")
{
	GameState s;

	SECTION("editor")
	{
		REQUIRE(s.editoractive == 0);
		REQUIRE(s.editorpathtype == 0);
	}

	SECTION("hawk")
	{
		REQUIRE(s.hawkyaw == 0.0f);
		REQUIRE(s.hawkcalldelay == 0.0f);
	}

	SECTION("cursor")
	{
		REQUIRE(s.oldmousecoordv == 0);
		REQUIRE(s.mousejump == false);
	}

	SECTION("console")
	{
		REQUIRE(s.consoleblink == false);
		REQUIRE(s.consoleblinkdelay == 0.0f);
	}

	SECTION("screen limits")
	{
		REQUIRE(s.maxscreenwidth == 3000.0f);
		REQUIRE(s.maxscreenheight == 3000.0f);
	}

	SECTION("timing")
	{
		REQUIRE(s.slomospeed == 0.0f);
		REQUIRE(s.fps == 0.0f);
	}

	SECTION("scoring")
	{
		REQUIRE(s.scoreadded == false);
		REQUIRE(s.againbonus == false);
	}
}

TEST_CASE("a freshly constructed GameState is unaffected by another instance", "[gamestate]")
{
	GameState a;
	GameState b;

	SECTION("scalar members are per instance")
	{
		a.editoractive = 3;
		a.editorpathtype = 1;
		a.hawkyaw = 2.5f;
		a.hawkcalldelay = 7.0f;
		a.oldmousecoordv = 12;
		a.mousejump = true;
		a.consoleblink = true;
		a.consoleblinkdelay = 0.3f;
		a.slomospeed = 0.25f;
		a.maxscreenwidth = 1920.0f;
		a.maxscreenheight = 1080.0f;
		a.scoreadded = true;
		a.againbonus = true;
		a.fps = 60.0f;

		REQUIRE(b.editoractive == 0);
		REQUIRE(b.editorpathtype == 0);
		REQUIRE(b.hawkyaw == 0.0f);
		REQUIRE(b.hawkcalldelay == 0.0f);
		REQUIRE(b.oldmousecoordv == 0);
		REQUIRE(b.mousejump == false);
		REQUIRE(b.consoleblink == false);
		REQUIRE(b.consoleblinkdelay == 0.0f);
		REQUIRE(b.slomospeed == 0.0f);
		REQUIRE(b.maxscreenwidth == 3000.0f);
		REQUIRE(b.maxscreenheight == 3000.0f);
		REQUIRE(b.scoreadded == false);
		REQUIRE(b.againbonus == false);
		REQUIRE(b.fps == 0.0f);

		GameState c;
		REQUIRE(c.editoractive == 0);
		REQUIRE(c.hawkyaw == 0.0f);
		REQUIRE(c.oldmousecoordv == 0);
		REQUIRE(c.mousejump == false);
		REQUIRE(c.consoleblink == false);
		REQUIRE(c.slomospeed == 0.0f);
		REQUIRE(c.maxscreenwidth == 3000.0f);
		REQUIRE(c.maxscreenheight == 3000.0f);
		REQUIRE(c.scoreadded == false);
		REQUIRE(c.againbonus == false);
		REQUIRE(c.fps == 0.0f);
	}

	SECTION("GameState is trivially destructible and needs no GL context")
	{
		// It holds scalars only; rendering resources stay out so this can be
		// constructed and destroyed in a plain unit test.
		REQUIRE(std::is_trivially_destructible<GameState>::value);
		REQUIRE(std::is_trivially_copyable<GameState>::value);
	}
}