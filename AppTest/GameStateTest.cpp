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

TEST_CASE("GameState tranche 3 members start at their historical global defaults", "[gamestate]")
{
	GameState s;

	SECTION("screen limits")
	{
		REQUIRE(s.minscreenwidth == 640.0f);
		REQUIRE(s.minscreenheight == 480.0f);
	}

	SECTION("session control")
	{
		REQUIRE(s.tryquit == 0);
		REQUIRE(s.endgame == 0);
	}

	SECTION("timing")
	{
		REQUIRE(s.realmultiplier == 0.0f);
		REQUIRE(s.hostiletime == 0.0f);
	}

	SECTION("screen flash")
	{
		REQUIRE(s.flashr == 0.0f);
		REQUIRE(s.flashg == 0.0f);
		REQUIRE(s.flashb == 0.0f);
		REQUIRE(s.flashdelay == 0);
	}

	SECTION("precipitation pacing")
	{
		REQUIRE(s.precipdelay == 0.0f);
	}

	SECTION("display options")
	{
		REQUIRE(s.velocityblur == false);
		REQUIRE(s.texttoggle == false);
		REQUIRE(s.damagedealt == 0.0f);
		REQUIRE(s.alwaysblur == false);
		REQUIRE(s.immediate == false);
		REQUIRE(s.floatjump == false);
		REQUIRE(s.autoslomo == false);
		REQUIRE(s.slomodelay == 0.0f);
		REQUIRE(s.showdamagebar == false);
		REQUIRE(s.showpoints == false);
		REQUIRE(s.invertmouse == false);
		REQUIRE(s.damageeffects == false);
		REQUIRE(s.cellophane == false);
		REQUIRE(s.foliage == false);
	}

	SECTION("motion blur")
	{
		REQUIRE(s.motionbluramount == 0.0f);
		REQUIRE(s.targetblurness == 0.0f);
	}

	SECTION("level loading and switching")
	{
		REQUIRE(s.oldenvironment == 0);
		REQUIRE(s.firstLoadDone == false);
	}

	SECTION("editor")
	{
		REQUIRE(s.editorsize == 0.0f);
	}

	SECTION("console")
	{
		REQUIRE(s.consolekey == 0);
		REQUIRE(s.consoleselected == 0);
	}

	SECTION("input")
	{
		REQUIRE(s.stereoreverse == false);
		REQUIRE(s.mousecoordh == 0);
		REQUIRE(s.mousecoordv == 0);
	}
}

TEST_CASE("tranche 3 GameState members are per instance", "[gamestate]")
{
	GameState a;
	GameState b;

	SECTION("writing one instance leaves the other at its defaults")
	{
		a.minscreenwidth = 800;
		a.minscreenheight = 600;
		a.tryquit = 1;
		a.endgame = 2;
		a.realmultiplier = 0.25f;
		a.hostiletime = 3.5f;
		a.flashr = 1;
		a.flashg = 0.5f;
		a.flashb = 0.25f;
		a.flashdelay = 7;
		a.precipdelay = 0.04f;
		a.velocityblur = true;
		a.texttoggle = true;
		a.damagedealt = 12.0f;
		a.alwaysblur = true;
		a.immediate = true;
		a.floatjump = true;
		a.autoslomo = true;
		a.slomodelay = 0.2f;
		a.showdamagebar = true;
		a.showpoints = true;
		a.invertmouse = true;
		a.damageeffects = true;
		a.cellophane = true;
		a.foliage = true;
		a.motionbluramount = 0.2f;
		a.targetblurness = 2.5f;
		a.oldenvironment = -4;
		a.firstLoadDone = true;
		a.editorsize = 1.5f;
		a.consolekey = 99;
		a.consoleselected = 6;
		a.stereoreverse = true;
		a.mousecoordh = 42;
		a.mousecoordv = 43;

		REQUIRE(b.minscreenwidth == 640.0f);
		REQUIRE(b.minscreenheight == 480.0f);
		REQUIRE(b.tryquit == 0);
		REQUIRE(b.endgame == 0);
		REQUIRE(b.realmultiplier == 0.0f);
		REQUIRE(b.hostiletime == 0.0f);
		REQUIRE(b.flashr == 0.0f);
		REQUIRE(b.flashg == 0.0f);
		REQUIRE(b.flashb == 0.0f);
		REQUIRE(b.flashdelay == 0);
		REQUIRE(b.precipdelay == 0.0f);
		REQUIRE(b.velocityblur == false);
		REQUIRE(b.texttoggle == false);
		REQUIRE(b.damagedealt == 0.0f);
		REQUIRE(b.alwaysblur == false);
		REQUIRE(b.immediate == false);
		REQUIRE(b.floatjump == false);
		REQUIRE(b.autoslomo == false);
		REQUIRE(b.slomodelay == 0.0f);
		REQUIRE(b.showdamagebar == false);
		REQUIRE(b.showpoints == false);
		REQUIRE(b.invertmouse == false);
		REQUIRE(b.damageeffects == false);
		REQUIRE(b.cellophane == false);
		REQUIRE(b.foliage == false);
		REQUIRE(b.motionbluramount == 0.0f);
		REQUIRE(b.targetblurness == 0.0f);
		REQUIRE(b.oldenvironment == 0);
		REQUIRE(b.firstLoadDone == false);
		REQUIRE(b.editorsize == 0.0f);
		REQUIRE(b.consolekey == 0);
		REQUIRE(b.consoleselected == 0);
		REQUIRE(b.stereoreverse == false);
		REQUIRE(b.mousecoordh == 0);
		REQUIRE(b.mousecoordv == 0);
	}

	SECTION("a third instance also starts clean")
	{
		a.minscreenwidth = 1;
		a.minscreenheight = 2;
		a.tryquit = 3;
		a.endgame = 4;
		a.realmultiplier = 5.0f;
		a.hostiletime = 6.0f;
		a.flashr = 7.0f;
		a.flashg = 8.0f;
		a.flashb = 9.0f;
		a.flashdelay = 10;
		a.precipdelay = 11.0f;
		a.velocityblur = true;
		a.texttoggle = true;
		a.damagedealt = 12.0f;
		a.alwaysblur = true;
		a.immediate = true;
		a.floatjump = true;
		a.autoslomo = true;
		a.slomodelay = 13.0f;
		a.showdamagebar = true;
		a.showpoints = true;
		a.invertmouse = true;
		a.damageeffects = true;
		a.cellophane = true;
		a.foliage = true;
		a.motionbluramount = 14.0f;
		a.targetblurness = 15.0f;
		a.oldenvironment = 16;
		a.firstLoadDone = true;
		a.editorsize = 17.0f;
		a.consolekey = 18;
		a.consoleselected = 19;
		a.stereoreverse = true;
		a.mousecoordh = 20;
		a.mousecoordv = 21;

		GameState c;
		REQUIRE(c.minscreenwidth == 640.0f);
		REQUIRE(c.minscreenheight == 480.0f);
		REQUIRE(c.tryquit == 0);
		REQUIRE(c.endgame == 0);
		REQUIRE(c.realmultiplier == 0.0f);
		REQUIRE(c.hostiletime == 0.0f);
		REQUIRE(c.flashr == 0.0f);
		REQUIRE(c.flashg == 0.0f);
		REQUIRE(c.flashb == 0.0f);
		REQUIRE(c.flashdelay == 0);
		REQUIRE(c.precipdelay == 0.0f);
		REQUIRE(c.velocityblur == false);
		REQUIRE(c.texttoggle == false);
		REQUIRE(c.damagedealt == 0.0f);
		REQUIRE(c.alwaysblur == false);
		REQUIRE(c.immediate == false);
		REQUIRE(c.floatjump == false);
		REQUIRE(c.autoslomo == false);
		REQUIRE(c.slomodelay == 0.0f);
		REQUIRE(c.showdamagebar == false);
		REQUIRE(c.showpoints == false);
		REQUIRE(c.invertmouse == false);
		REQUIRE(c.damageeffects == false);
		REQUIRE(c.cellophane == false);
		REQUIRE(c.foliage == false);
		REQUIRE(c.motionbluramount == 0.0f);
		REQUIRE(c.targetblurness == 0.0f);
		REQUIRE(c.oldenvironment == 0);
		REQUIRE(c.firstLoadDone == false);
		REQUIRE(c.editorsize == 0.0f);
		REQUIRE(c.consolekey == 0);
		REQUIRE(c.consoleselected == 0);
		REQUIRE(c.stereoreverse == false);
		REQUIRE(c.mousecoordh == 0);
		REQUIRE(c.mousecoordv == 0);
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(a.minscreenwidth), float>::value);
		REQUIRE(std::is_same<decltype(a.minscreenheight), float>::value);
		REQUIRE(std::is_same<decltype(a.tryquit), int>::value);
		REQUIRE(std::is_same<decltype(a.endgame), int>::value);
		REQUIRE(std::is_same<decltype(a.realmultiplier), float>::value);
		REQUIRE(std::is_same<decltype(a.hostiletime), float>::value);
		REQUIRE(std::is_same<decltype(a.flashr), float>::value);
		REQUIRE(std::is_same<decltype(a.flashg), float>::value);
		REQUIRE(std::is_same<decltype(a.flashb), float>::value);
		REQUIRE(std::is_same<decltype(a.flashdelay), int>::value);
		REQUIRE(std::is_same<decltype(a.precipdelay), float>::value);
		REQUIRE(std::is_same<decltype(a.velocityblur), bool>::value);
		REQUIRE(std::is_same<decltype(a.texttoggle), bool>::value);
		REQUIRE(std::is_same<decltype(a.damagedealt), float>::value);
		REQUIRE(std::is_same<decltype(a.alwaysblur), bool>::value);
		REQUIRE(std::is_same<decltype(a.immediate), bool>::value);
		REQUIRE(std::is_same<decltype(a.floatjump), bool>::value);
		REQUIRE(std::is_same<decltype(a.autoslomo), bool>::value);
		REQUIRE(std::is_same<decltype(a.slomodelay), float>::value);
		REQUIRE(std::is_same<decltype(a.showdamagebar), bool>::value);
		REQUIRE(std::is_same<decltype(a.showpoints), bool>::value);
		REQUIRE(std::is_same<decltype(a.invertmouse), bool>::value);
		REQUIRE(std::is_same<decltype(a.damageeffects), bool>::value);
		REQUIRE(std::is_same<decltype(a.cellophane), bool>::value);
		REQUIRE(std::is_same<decltype(a.foliage), bool>::value);
		REQUIRE(std::is_same<decltype(a.motionbluramount), float>::value);
		REQUIRE(std::is_same<decltype(a.targetblurness), float>::value);
		REQUIRE(std::is_same<decltype(a.oldenvironment), int>::value);
		REQUIRE(std::is_same<decltype(a.firstLoadDone), bool>::value);
		REQUIRE(std::is_same<decltype(a.editorsize), float>::value);
		REQUIRE(std::is_same<decltype(a.consolekey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.consoleselected), unsigned>::value);
		REQUIRE(std::is_same<decltype(a.stereoreverse), bool>::value);
		REQUIRE(std::is_same<decltype(a.mousecoordh), int>::value);
		REQUIRE(std::is_same<decltype(a.mousecoordv), int>::value);
	}
}