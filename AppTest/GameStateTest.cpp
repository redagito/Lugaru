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

TEST_CASE("GameState tranche 2 members start at their historical global defaults", "[gamestate]")
{
	GameState s;

	SECTION("campaign choice")
	{
		REQUIRE(s.whichchoice == 0);
	}

	SECTION("time scale")
	{
		REQUIRE(s.gamespeed == 0.0f);
		REQUIRE(s.oldgamespeed == 0.0f);
	}

	SECTION("level loading")
	{
		REQUIRE(s.loading == 0);
		REQUIRE(s.stillloading == false);
		REQUIRE(s.visibleloading == false);
	}

	SECTION("level switching")
	{
		REQUIRE(s.changedelay == 0.0f);
	}

	SECTION("level clock")
	{
		REQUIRE(s.loadtime == 0.0f);
		REQUIRE(s.leveltime == 0.0f);
		REQUIRE(s.wonleveltime == 0.0f);
	}

	SECTION("freeze")
	{
		REQUIRE(s.freeze == false);
		REQUIRE(s.winfreeze == false);
	}

	SECTION("tutorial gating")
	{
		REQUIRE(s.cananger == false);
	}
}

TEST_CASE("tranche 2 GameState members are per instance", "[gamestate]")
{
	GameState a;
	GameState b;

	SECTION("writing one instance leaves the other at its defaults")
	{
		a.whichchoice = 3;
		a.gamespeed = 0.5f;
		a.oldgamespeed = 0.25f;
		a.loading = 4;
		a.stillloading = true;
		a.visibleloading = true;
		a.changedelay = -999.0f;
		a.loadtime = 12.5f;
		a.leveltime = 42.0f;
		a.wonleveltime = 41.5f;
		a.freeze = true;
		a.winfreeze = true;
		a.cananger = true;

		REQUIRE(b.whichchoice == 0);
		REQUIRE(b.gamespeed == 0.0f);
		REQUIRE(b.oldgamespeed == 0.0f);
		REQUIRE(b.loading == 0);
		REQUIRE(b.stillloading == false);
		REQUIRE(b.visibleloading == false);
		REQUIRE(b.changedelay == 0.0f);
		REQUIRE(b.loadtime == 0.0f);
		REQUIRE(b.leveltime == 0.0f);
		REQUIRE(b.wonleveltime == 0.0f);
		REQUIRE(b.freeze == false);
		REQUIRE(b.winfreeze == false);
		REQUIRE(b.cananger == false);
	}

	SECTION("a third instance also starts clean")
	{
		a.whichchoice = 2;
		a.gamespeed = 0.1f;
		a.oldgamespeed = 0.1f;
		a.loading = 3;
		a.stillloading = true;
		a.visibleloading = true;
		a.changedelay = 0.1f;
		a.loadtime = 1.0f;
		a.leveltime = 2.0f;
		a.wonleveltime = 3.0f;
		a.freeze = true;
		a.winfreeze = true;
		a.cananger = true;

		GameState c;
		REQUIRE(c.whichchoice == 0);
		REQUIRE(c.gamespeed == 0.0f);
		REQUIRE(c.oldgamespeed == 0.0f);
		REQUIRE(c.loading == 0);
		REQUIRE(c.stillloading == false);
		REQUIRE(c.visibleloading == false);
		REQUIRE(c.changedelay == 0.0f);
		REQUIRE(c.loadtime == 0.0f);
		REQUIRE(c.leveltime == 0.0f);
		REQUIRE(c.wonleveltime == 0.0f);
		REQUIRE(c.freeze == false);
		REQUIRE(c.winfreeze == false);
		REQUIRE(c.cananger == false);
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(a.whichchoice), int>::value);
		REQUIRE(std::is_same<decltype(a.gamespeed), float>::value);
		REQUIRE(std::is_same<decltype(a.oldgamespeed), float>::value);
		REQUIRE(std::is_same<decltype(a.loading), int>::value);
		REQUIRE(std::is_same<decltype(a.stillloading), bool>::value);
		REQUIRE(std::is_same<decltype(a.visibleloading), bool>::value);
		REQUIRE(std::is_same<decltype(a.changedelay), float>::value);
		REQUIRE(std::is_same<decltype(a.loadtime), float>::value);
		REQUIRE(std::is_same<decltype(a.leveltime), float>::value);
		REQUIRE(std::is_same<decltype(a.wonleveltime), float>::value);
		REQUIRE(std::is_same<decltype(a.freeze), bool>::value);
		REQUIRE(std::is_same<decltype(a.winfreeze), bool>::value);
		REQUIRE(std::is_same<decltype(a.cananger), bool>::value);
	}
}