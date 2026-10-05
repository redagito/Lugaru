// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "GameGlobals.h"
#include "Globals.h"
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
		// It holds scalars and plain-data arrays, no rendering resources, so this
		// can be constructed and destroyed in a plain unit test.
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

	SECTION("scoring")
	{
		REQUIRE(s.damagedealt == 0.0f);
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

// Tranche 4 covers the remaining scalar globals from App/include/Globals.h and
// App/include/GameGlobals.h. Each member's default is pinned to the literal the
// global it came from was initialised with, and its declared type to the type
// that global declared.
TEST_CASE("GameState tranche 4 members start at their historical global defaults", "[gamestate]")
{
	GameState s;

	SECTION("keybinds")
	{
		REQUIRE(s.crouchkey == 0);
		REQUIRE(s.jumpkey == 0);
		REQUIRE(s.forwardkey == 0);
		REQUIRE(s.backkey == 0);
		REQUIRE(s.leftkey == 0);
		REQUIRE(s.rightkey == 0);
		REQUIRE(s.drawkey == 0);
		REQUIRE(s.throwkey == 0);
		REQUIRE(s.attackkey == 0);
	}

	SECTION("audio")
	{
		REQUIRE(s.volume == 0.0f);
		REQUIRE(s.musictoggle == false);
		REQUIRE(s.ambientsound == false);
	}

	SECTION("display settings")
	{
		REQUIRE(s.fullscreen == false);
		REQUIRE(s.ismotionblur == false);
		REQUIRE(s.usermousesensitivity == 0.0f);
		REQUIRE(s.stereoseparation == 0.05f);
	}

	SECTION("texture budget")
	{
		// kTextureSize looks like a compile-time constant, but Game::LoadStuff
		// assigns it from the runtime detail setting, so it is a member rather
		// than a constant, and starts at zero rather than at any of the values
		// it is later given.
		REQUIRE(s.kTextureSize == 0);
	}

	SECTION("skybox")
	{
		REQUIRE(s.skyboxr == 0.0f);
		REQUIRE(s.skyboxg == 0.0f);
		REQUIRE(s.skyboxb == 0.0f);
		REQUIRE(s.skyboxlightr == 0.0f);
		REQUIRE(s.skyboxlightg == 0.0f);
		REQUIRE(s.skyboxlightb == 0.0f);
	}

	SECTION("mouse look")
	{
		REQUIRE(s.deltah == 0.0f);
		REQUIRE(s.deltav == 0.0f);
	}

	SECTION("world map")
	{
		REQUIRE(s.mapradius == 0.0f);
		REQUIRE(s.maptype == 0);
	}

	SECTION("editor camera")
	{
		REQUIRE(s.editoryaw == 0.0f);
		REQUIRE(s.editorpitch == 0.0f);
	}

	SECTION("challenge progression")
	{
		REQUIRE(s.numchallengelevels == 0);
	}

	SECTION("session start")
	{
		REQUIRE(s.gamestarted == false);
	}

	SECTION("camera wobble")
	{
		REQUIRE(s.woozy == 0.0f);
	}

	SECTION("smoke animation")
	{
		REQUIRE(s.smoketex == 0.0f);
	}

	SECTION("object culling")
	{
		REQUIRE(s.playerdist == 0.0f);
	}

	SECTION("tutorial gating")
	{
		REQUIRE(s.canattack == false);
		REQUIRE(s.reversaltrain == false);
	}
}

TEST_CASE("tranche 4 GameState members are per instance", "[gamestate]")
{
	GameState a;
	GameState b;

	SECTION("writing one instance leaves the other at its defaults")
	{
		a.crouchkey = 11;
		a.jumpkey = 12;
		a.forwardkey = 13;
		a.backkey = 14;
		a.leftkey = 15;
		a.rightkey = 16;
		a.drawkey = 17;
		a.throwkey = 18;
		a.attackkey = 19;
		a.volume = 0.5f;
		a.musictoggle = true;
		a.ambientsound = true;
		a.fullscreen = true;
		a.ismotionblur = true;
		a.usermousesensitivity = 2.0f;
		a.stereoseparation = 0.5f;
		a.kTextureSize = 1024;
		a.skyboxr = 0.1f;
		a.skyboxg = 0.2f;
		a.skyboxb = 0.3f;
		a.skyboxlightr = 0.4f;
		a.skyboxlightg = 0.5f;
		a.skyboxlightb = 0.6f;
		a.deltah = 1.5f;
		a.deltav = 2.5f;
		a.mapradius = 3.5f;
		a.maptype = 2;
		a.editoryaw = 4.5f;
		a.editorpitch = 5.5f;
		a.numchallengelevels = 14;
		a.gamestarted = true;
		a.woozy = 6.5f;
		a.smoketex = 7.5f;
		a.playerdist = 8.5f;
		a.canattack = true;
		a.reversaltrain = true;

		REQUIRE(b.crouchkey == 0);
		REQUIRE(b.jumpkey == 0);
		REQUIRE(b.forwardkey == 0);
		REQUIRE(b.backkey == 0);
		REQUIRE(b.leftkey == 0);
		REQUIRE(b.rightkey == 0);
		REQUIRE(b.drawkey == 0);
		REQUIRE(b.throwkey == 0);
		REQUIRE(b.attackkey == 0);
		REQUIRE(b.volume == 0.0f);
		REQUIRE(b.musictoggle == false);
		REQUIRE(b.ambientsound == false);
		REQUIRE(b.fullscreen == false);
		REQUIRE(b.ismotionblur == false);
		REQUIRE(b.usermousesensitivity == 0.0f);
		REQUIRE(b.stereoseparation == 0.05f);
		REQUIRE(b.kTextureSize == 0);
		REQUIRE(b.skyboxr == 0.0f);
		REQUIRE(b.skyboxg == 0.0f);
		REQUIRE(b.skyboxb == 0.0f);
		REQUIRE(b.skyboxlightr == 0.0f);
		REQUIRE(b.skyboxlightg == 0.0f);
		REQUIRE(b.skyboxlightb == 0.0f);
		REQUIRE(b.deltah == 0.0f);
		REQUIRE(b.deltav == 0.0f);
		REQUIRE(b.mapradius == 0.0f);
		REQUIRE(b.maptype == 0);
		REQUIRE(b.editoryaw == 0.0f);
		REQUIRE(b.editorpitch == 0.0f);
		REQUIRE(b.numchallengelevels == 0);
		REQUIRE(b.gamestarted == false);
		REQUIRE(b.woozy == 0.0f);
		REQUIRE(b.smoketex == 0.0f);
		REQUIRE(b.playerdist == 0.0f);
		REQUIRE(b.canattack == false);
		REQUIRE(b.reversaltrain == false);
	}

	SECTION("a third instance also starts clean")
	{
		a.crouchkey = 101;
		a.jumpkey = 102;
		a.forwardkey = 103;
		a.backkey = 104;
		a.leftkey = 105;
		a.rightkey = 106;
		a.drawkey = 107;
		a.throwkey = 108;
		a.attackkey = 109;
		a.volume = 1.5f;
		a.musictoggle = true;
		a.ambientsound = true;
		a.fullscreen = true;
		a.ismotionblur = true;
		a.usermousesensitivity = 3.0f;
		a.stereoseparation = 1.5f;
		a.kTextureSize = 512;
		a.skyboxr = 1.1f;
		a.skyboxg = 1.2f;
		a.skyboxb = 1.3f;
		a.skyboxlightr = 1.4f;
		a.skyboxlightg = 1.5f;
		a.skyboxlightb = 1.6f;
		a.deltah = 2.5f;
		a.deltav = 3.5f;
		a.mapradius = 4.5f;
		a.maptype = 3;
		a.editoryaw = 5.5f;
		a.editorpitch = 6.5f;
		a.numchallengelevels = 7;
		a.gamestarted = true;
		a.woozy = 7.5f;
		a.smoketex = 8.5f;
		a.playerdist = 9.5f;
		a.canattack = true;
		a.reversaltrain = true;

		GameState c;
		REQUIRE(c.crouchkey == 0);
		REQUIRE(c.jumpkey == 0);
		REQUIRE(c.forwardkey == 0);
		REQUIRE(c.backkey == 0);
		REQUIRE(c.leftkey == 0);
		REQUIRE(c.rightkey == 0);
		REQUIRE(c.drawkey == 0);
		REQUIRE(c.throwkey == 0);
		REQUIRE(c.attackkey == 0);
		REQUIRE(c.volume == 0.0f);
		REQUIRE(c.musictoggle == false);
		REQUIRE(c.ambientsound == false);
		REQUIRE(c.fullscreen == false);
		REQUIRE(c.ismotionblur == false);
		REQUIRE(c.usermousesensitivity == 0.0f);
		REQUIRE(c.stereoseparation == 0.05f);
		REQUIRE(c.kTextureSize == 0);
		REQUIRE(c.skyboxr == 0.0f);
		REQUIRE(c.skyboxg == 0.0f);
		REQUIRE(c.skyboxb == 0.0f);
		REQUIRE(c.skyboxlightr == 0.0f);
		REQUIRE(c.skyboxlightg == 0.0f);
		REQUIRE(c.skyboxlightb == 0.0f);
		REQUIRE(c.deltah == 0.0f);
		REQUIRE(c.deltav == 0.0f);
		REQUIRE(c.mapradius == 0.0f);
		REQUIRE(c.maptype == 0);
		REQUIRE(c.editoryaw == 0.0f);
		REQUIRE(c.editorpitch == 0.0f);
		REQUIRE(c.numchallengelevels == 0);
		REQUIRE(c.gamestarted == false);
		REQUIRE(c.woozy == 0.0f);
		REQUIRE(c.smoketex == 0.0f);
		REQUIRE(c.playerdist == 0.0f);
		REQUIRE(c.canattack == false);
		REQUIRE(c.reversaltrain == false);
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(a.crouchkey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.jumpkey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.forwardkey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.backkey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.leftkey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.rightkey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.drawkey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.throwkey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.attackkey), unsigned short>::value);
		REQUIRE(std::is_same<decltype(a.volume), float>::value);
		REQUIRE(std::is_same<decltype(a.musictoggle), bool>::value);
		REQUIRE(std::is_same<decltype(a.ambientsound), bool>::value);
		REQUIRE(std::is_same<decltype(a.fullscreen), bool>::value);
		REQUIRE(std::is_same<decltype(a.ismotionblur), bool>::value);
		REQUIRE(std::is_same<decltype(a.usermousesensitivity), float>::value);
		REQUIRE(std::is_same<decltype(a.stereoseparation), float>::value);
		REQUIRE(std::is_same<decltype(a.kTextureSize), int>::value);
		REQUIRE(std::is_same<decltype(a.skyboxr), float>::value);
		REQUIRE(std::is_same<decltype(a.skyboxg), float>::value);
		REQUIRE(std::is_same<decltype(a.skyboxb), float>::value);
		REQUIRE(std::is_same<decltype(a.skyboxlightr), float>::value);
		REQUIRE(std::is_same<decltype(a.skyboxlightg), float>::value);
		REQUIRE(std::is_same<decltype(a.skyboxlightb), float>::value);
		REQUIRE(std::is_same<decltype(a.deltah), float>::value);
		REQUIRE(std::is_same<decltype(a.deltav), float>::value);
		REQUIRE(std::is_same<decltype(a.mapradius), float>::value);
		REQUIRE(std::is_same<decltype(a.maptype), int>::value);
		REQUIRE(std::is_same<decltype(a.editoryaw), float>::value);
		REQUIRE(std::is_same<decltype(a.editorpitch), float>::value);
		REQUIRE(std::is_same<decltype(a.numchallengelevels), int>::value);
		REQUIRE(std::is_same<decltype(a.gamestarted), bool>::value);
		REQUIRE(std::is_same<decltype(a.woozy), float>::value);
		REQUIRE(std::is_same<decltype(a.smoketex), float>::value);
		REQUIRE(std::is_same<decltype(a.playerdist), float>::value);
		REQUIRE(std::is_same<decltype(a.canattack), bool>::value);
		REQUIRE(std::is_same<decltype(a.reversaltrain), bool>::value);
	}
}

// Tranche 5 covers twenty-six more scalar globals from App/include/Globals.h
// and App/include/GameGlobals.h. Each member's default is the literal the
// global it came from was initialised with, and its declared type is the type
// that global declared. The literals were first pinned by reading the globals
// themselves, while they were still around to be compared against.
//
// Types are pinned next to the values on purpose. A value comparison such as
// `REQUIRE(x == 0)` holds just as happily for an int as for a float, so on its
// own it would let a member change type unnoticed.
TEST_CASE("GameState tranche 5 members start at their historical global defaults", "[gamestate]")
{
	GameState s;

	SECTION("display options")
	{
		REQUIRE(s.decalstoggle == false);
	}

	SECTION("motion blur")
	{
		REQUIRE(s.blurness == 0.0f);
	}

	SECTION("wind")
	{
		REQUIRE(s.windvar == 0.0f);
	}

	SECTION("terrain texturing and physics")
	{
		REQUIRE(s.texscale == 0.0f);
		REQUIRE(s.gravity == 0.0f);
	}

	SECTION("time scale")
	{
		REQUIRE(s.slomo == 0);
	}

	SECTION("screen darkening from blood loss")
	{
		REQUIRE(s.blackout == 0.0f);
	}

	SECTION("screen flash")
	{
		REQUIRE(s.flashamount == 0.0f);
	}

	SECTION("level hostility")
	{
		REQUIRE(s.hostile == 0);
	}

	SECTION("skybox")
	{
		REQUIRE(s.skyboxtexture == false);
	}

	SECTION("devtools")
	{
		REQUIRE(s.devtools == false);
	}

	SECTION("texture detail")
	{
		REQUIRE(s.realtexdetail == 0.0f);
	}

	SECTION("options menu choices, not yet applied")
	{
		REQUIRE(s.newdetail == 0);
		REQUIRE(s.newscreenwidth == 0);
		REQUIRE(s.newscreenheight == 0);
	}

	SECTION("session control")
	{
		REQUIRE(s.gameon == false);
	}

	SECTION("editor")
	{
		REQUIRE(s.editorenabled == false);
		REQUIRE(s.editortype == 0);
		REQUIRE(s.pathpointselected == 0);
	}

	SECTION("keybind capture")
	{
		REQUIRE(s.keyselect == 0);
	}

	SECTION("audio")
	{
		REQUIRE(s.musictype == 0);
	}

	SECTION("level loading")
	{
		REQUIRE(s.stealthloading == false);
	}

	SECTION("free camera")
	{
		REQUIRE(s.cameramode == false);
	}

	SECTION("console")
	{
		REQUIRE(s.console == false);
	}

	SECTION("level switching")
	{
		REQUIRE(s.targetlevel == 0);
	}

	SECTION("text input")
	{
		REQUIRE(s.waiting == false);
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(s.decalstoggle), bool>::value);
		REQUIRE(std::is_same<decltype(s.blurness), float>::value);
		REQUIRE(std::is_same<decltype(s.windvar), float>::value);
		REQUIRE(std::is_same<decltype(s.texscale), float>::value);
		REQUIRE(std::is_same<decltype(s.gravity), float>::value);
		REQUIRE(std::is_same<decltype(s.slomo), int>::value);
		REQUIRE(std::is_same<decltype(s.blackout), float>::value);
		REQUIRE(std::is_same<decltype(s.flashamount), float>::value);
		REQUIRE(std::is_same<decltype(s.hostile), int>::value);
		REQUIRE(std::is_same<decltype(s.skyboxtexture), bool>::value);
		REQUIRE(std::is_same<decltype(s.devtools), bool>::value);
		REQUIRE(std::is_same<decltype(s.realtexdetail), float>::value);
		REQUIRE(std::is_same<decltype(s.newdetail), int>::value);
		REQUIRE(std::is_same<decltype(s.newscreenwidth), int>::value);
		REQUIRE(std::is_same<decltype(s.newscreenheight), int>::value);
		REQUIRE(std::is_same<decltype(s.gameon), bool>::value);
		REQUIRE(std::is_same<decltype(s.editorenabled), bool>::value);
		REQUIRE(std::is_same<decltype(s.editortype), int>::value);
		REQUIRE(std::is_same<decltype(s.pathpointselected), int>::value);
		REQUIRE(std::is_same<decltype(s.keyselect), int>::value);
		REQUIRE(std::is_same<decltype(s.musictype), int>::value);
		REQUIRE(std::is_same<decltype(s.stealthloading), bool>::value);
		REQUIRE(std::is_same<decltype(s.cameramode), bool>::value);
		REQUIRE(std::is_same<decltype(s.console), bool>::value);
		REQUIRE(std::is_same<decltype(s.targetlevel), int>::value);
		REQUIRE(std::is_same<decltype(s.waiting), bool>::value);
	}
}

TEST_CASE("tranche 5 GameState members are per instance", "[gamestate]")
{
	GameState a;
	GameState b;

	SECTION("writing one instance leaves the other at its defaults")
	{
		a.decalstoggle = true;
		a.blurness = 1.5f;
		a.windvar = 2.5f;
		a.texscale = 3.5f;
		a.gravity = -10.0f;
		a.slomo = 1;
		a.blackout = 4.5f;
		a.flashamount = 5.5f;
		a.hostile = 1;
		a.skyboxtexture = true;
		a.devtools = true;
		a.realtexdetail = 2.0f;
		a.newdetail = 2;
		a.newscreenwidth = 1920;
		a.newscreenheight = 1080;
		a.gameon = true;
		a.editorenabled = true;
		a.editortype = 4;
		a.pathpointselected = 7;
		a.keyselect = 3;
		a.musictype = 5;
		a.stealthloading = true;
		a.cameramode = true;
		a.console = true;
		a.targetlevel = 9;
		a.waiting = true;

		// Every one of the twenty-six was seeded to a value its default does not
		// hold, so each assertion below is an observation of a value the writer
		// never touched rather than a restatement of the default.
		REQUIRE(b.decalstoggle == false);
		REQUIRE(b.blurness == 0.0f);
		REQUIRE(b.windvar == 0.0f);
		REQUIRE(b.texscale == 0.0f);
		REQUIRE(b.gravity == 0.0f);
		REQUIRE(b.slomo == 0);
		REQUIRE(b.blackout == 0.0f);
		REQUIRE(b.flashamount == 0.0f);
		REQUIRE(b.hostile == 0);
		REQUIRE(b.skyboxtexture == false);
		REQUIRE(b.devtools == false);
		REQUIRE(b.realtexdetail == 0.0f);
		REQUIRE(b.newdetail == 0);
		REQUIRE(b.newscreenwidth == 0);
		REQUIRE(b.newscreenheight == 0);
		REQUIRE(b.gameon == false);
		REQUIRE(b.editorenabled == false);
		REQUIRE(b.editortype == 0);
		REQUIRE(b.pathpointselected == 0);
		REQUIRE(b.keyselect == 0);
		REQUIRE(b.musictype == 0);
		REQUIRE(b.stealthloading == false);
		REQUIRE(b.cameramode == false);
		REQUIRE(b.console == false);
		REQUIRE(b.targetlevel == 0);
		REQUIRE(b.waiting == false);
	}

	SECTION("a third instance also starts clean")
	{
		a.decalstoggle = true;
		a.blurness = 6.5f;
		a.windvar = 7.5f;
		a.texscale = 8.5f;
		a.gravity = -20.0f;
		a.slomo = 1;
		a.blackout = 9.5f;
		a.flashamount = 10.5f;
		a.hostile = 1;
		a.skyboxtexture = true;
		a.devtools = true;
		a.realtexdetail = 4.0f;
		a.newdetail = 1;
		a.newscreenwidth = 1280;
		a.newscreenheight = 720;
		a.gameon = true;
		a.editorenabled = true;
		a.editortype = 6;
		a.pathpointselected = 11;
		a.keyselect = 6;
		a.musictype = 8;
		a.stealthloading = true;
		a.cameramode = true;
		a.console = true;
		a.targetlevel = 12;
		a.waiting = true;

		GameState c;
		REQUIRE(c.decalstoggle == false);
		REQUIRE(c.blurness == 0.0f);
		REQUIRE(c.windvar == 0.0f);
		REQUIRE(c.texscale == 0.0f);
		REQUIRE(c.gravity == 0.0f);
		REQUIRE(c.slomo == 0);
		REQUIRE(c.blackout == 0.0f);
		REQUIRE(c.flashamount == 0.0f);
		REQUIRE(c.hostile == 0);
		REQUIRE(c.skyboxtexture == false);
		REQUIRE(c.devtools == false);
		REQUIRE(c.realtexdetail == 0.0f);
		REQUIRE(c.newdetail == 0);
		REQUIRE(c.newscreenwidth == 0);
		REQUIRE(c.newscreenheight == 0);
		REQUIRE(c.gameon == false);
		REQUIRE(c.editorenabled == false);
		REQUIRE(c.editortype == 0);
		REQUIRE(c.pathpointselected == 0);
		REQUIRE(c.keyselect == 0);
		REQUIRE(c.musictype == 0);
		REQUIRE(c.stealthloading == false);
		REQUIRE(c.cameramode == false);
		REQUIRE(c.console == false);
		REQUIRE(c.targetlevel == 0);
		REQUIRE(c.waiting == false);
	}
}

// Tranche 6 covers fifteen more scalar globals from App/include/Globals.h and
// App/include/GameGlobals.h. Each member's default is the literal the global it
// came from was initialised with, and its declared type is the type that global
// declared. Those literals were first pinned by reading the globals themselves,
// while they were still around to be compared against.
//
// Types are pinned next to the values on purpose. A value comparison such as
// `REQUIRE(x == 0)` holds just as happily for an int as for a float, so on its
// own it would let a member change type unnoticed.
TEST_CASE("GameState tranche 6 members start at their historical global defaults", "[gamestate]")
{
	GameState s;

	SECTION("game difficulty")
	{
		REQUIRE(s.difficulty == 0);
	}

	SECTION("window resolution")
	{
		REQUIRE(s.screenwidth == 0.0f);
		REQUIRE(s.screenheight == 0.0f);
	}

	SECTION("view distance and fading")
	{
		REQUIRE(s.viewdistance == 0.0f);
		REQUIRE(s.fadestart == 0.0f);
	}

	SECTION("level theme")
	{
		REQUIRE(s.environment == 0);
	}

	SECTION("graphics detail")
	{
		REQUIRE(s.detail == 0);
	}

	SECTION("skin texture resolution")
	{
		REQUIRE(s.texdetail == 0.0f);
	}

	SECTION("blood")
	{
		REQUIRE(s.bloodtoggle == 0);
	}

	SECTION("camera shake")
	{
		REQUIRE(s.camerashake == 0.0f);
	}

	SECTION("texture filtering")
	{
		REQUIRE(s.trilinear == false);
	}

	SECTION("menu state")
	{
		REQUIRE(s.mainmenu == 0);
	}

	SECTION("main menu highlight")
	{
		REQUIRE(s.selected == 0);
	}

	SECTION("camera orientation")
	{
		REQUIRE(s.yaw == 0.0f);
		REQUIRE(s.pitch == 0.0f);
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(s.difficulty), int>::value);
		REQUIRE(std::is_same<decltype(s.screenwidth), float>::value);
		REQUIRE(std::is_same<decltype(s.screenheight), float>::value);
		REQUIRE(std::is_same<decltype(s.viewdistance), float>::value);
		REQUIRE(std::is_same<decltype(s.fadestart), float>::value);
		REQUIRE(std::is_same<decltype(s.environment), int>::value);
		REQUIRE(std::is_same<decltype(s.detail), int>::value);
		REQUIRE(std::is_same<decltype(s.texdetail), float>::value);
		REQUIRE(std::is_same<decltype(s.bloodtoggle), int>::value);
		REQUIRE(std::is_same<decltype(s.camerashake), float>::value);
		REQUIRE(std::is_same<decltype(s.trilinear), bool>::value);
		REQUIRE(std::is_same<decltype(s.mainmenu), int>::value);
		REQUIRE(std::is_same<decltype(s.selected), int>::value);
		REQUIRE(std::is_same<decltype(s.yaw), float>::value);
		REQUIRE(std::is_same<decltype(s.pitch), float>::value);
	}
}

TEST_CASE("tranche 6 GameState members are per instance", "[gamestate]")
{
	GameState a;
	GameState b;

	SECTION("writing one instance leaves the other at its defaults")
	{
		a.difficulty = 2;
		a.screenwidth = 1920.0f;
		a.screenheight = 1080.0f;
		a.viewdistance = 250.0f;
		a.fadestart = 0.6f;
		a.environment = 2;
		a.detail = 1;
		a.texdetail = 4.0f;
		a.bloodtoggle = 2;
		a.camerashake = 0.8f;
		a.trilinear = true;
		a.mainmenu = 5;
		a.selected = -1;
		a.yaw = 137.0f;
		a.pitch = -45.0f;

		// Every one of the fifteen was seeded to a value its default does not
		// hold, so each assertion below is an observation of a value the writer
		// never touched rather than a restatement of the default.
		REQUIRE(b.difficulty == 0);
		REQUIRE(b.screenwidth == 0.0f);
		REQUIRE(b.screenheight == 0.0f);
		REQUIRE(b.viewdistance == 0.0f);
		REQUIRE(b.fadestart == 0.0f);
		REQUIRE(b.environment == 0);
		REQUIRE(b.detail == 0);
		REQUIRE(b.texdetail == 0.0f);
		REQUIRE(b.bloodtoggle == 0);
		REQUIRE(b.camerashake == 0.0f);
		REQUIRE(b.trilinear == false);
		REQUIRE(b.mainmenu == 0);
		REQUIRE(b.selected == 0);
		REQUIRE(b.yaw == 0.0f);
		REQUIRE(b.pitch == 0.0f);
	}

	SECTION("a third instance also starts clean")
	{
		a.difficulty = 1;
		a.screenwidth = 800.0f;
		a.screenheight = 600.0f;
		a.viewdistance = 100.0f;
		a.fadestart = 0.75f;
		a.environment = 1;
		a.detail = 2;
		a.texdetail = 2.0f;
		a.bloodtoggle = 1;
		a.camerashake = 0.4f;
		a.trilinear = true;
		a.mainmenu = 18;
		a.selected = 3;
		a.yaw = -90.0f;
		a.pitch = 90.0f;

		GameState c;
		REQUIRE(c.difficulty == 0);
		REQUIRE(c.screenwidth == 0.0f);
		REQUIRE(c.screenheight == 0.0f);
		REQUIRE(c.viewdistance == 0.0f);
		REQUIRE(c.fadestart == 0.0f);
		REQUIRE(c.environment == 0);
		REQUIRE(c.detail == 0);
		REQUIRE(c.texdetail == 0.0f);
		REQUIRE(c.bloodtoggle == 0);
		REQUIRE(c.camerashake == 0.0f);
		REQUIRE(c.trilinear == false);
		REQUIRE(c.mainmenu == 0);
		REQUIRE(c.selected == 0);
		REQUIRE(c.yaw == 0.0f);
		REQUIRE(c.pitch == 0.0f);
	}
}

// Tranche 7 covers one global, `multiplier`: the per-tick time scale. The
// member's default is the literal the global it came from was initialised
// with, and its declared type is the type that global declared. That literal
// was first pinned by reading the global itself, while it was still around to
// be compared against.
//
// The type is pinned alongside the value because a comparison such as
// `REQUIRE(x == 0)` holds just as happily for an int as for a float, and the
// test projects compile with /wd4244, which suppresses the float-to-int
// conversion warning that would otherwise be the only hint of a type slip.
TEST_CASE("GameState tranche 7 members start at their historical global defaults", "[gamestate]")
{
	GameState s;

	SECTION("per-tick time scale")
	{
		REQUIRE(s.multiplier == 0.0f);
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(s.multiplier), float>::value);
	}
}

TEST_CASE("tranche 7 GameState members are per instance", "[gamestate]")
{
	GameState a;
	GameState b;

	SECTION("writing one instance leaves the other at its defaults")
	{
		a.multiplier = 0.016f;

		REQUIRE(b.multiplier == 0.0f);
	}

	SECTION("a third instance also starts clean")
	{
		a.multiplier = 0.5f;

		GameState c;
		REQUIRE(c.multiplier == 0.0f);
	}
}

// Tranche 8 covers the four globals that together hold the editor's pathfinding
// waypoint graph: `numpathpoints`, `pathpoint`, `numpathpointconnect` and
// `pathpointconnect`. They index each other - the connect counts and the connect
// table are both keyed by a path point, and the table's entries are indices back
// into the point list - so they are pinned here as one group and moved as one
// group. Splitting them would leave the graph half in each place.
//
// These are the assertions the previous commit made against the globals
// themselves, re-pointed at the members and with nothing dropped. The values were
// read from the globals while they were still there, so they are observed rather
// than assumed, and they sit next to `pathpointselected` because the editor's
// link and delete commands key off that one.
//
// `pathpoint` was declared with no initialiser, unlike its two siblings which
// said `= {}`. It still started all zeroes, because Vector3 gives x, y and z
// default member initialisers, so default-constructing the array ran them for
// every element. That is pinned by reading all thirty points rather than being
// taken on trust, and the member spells the same intent out as `= {}` so the
// zeroing does not depend on Vector3 keeping its default member initialisers.
//
// The declared types are pinned whole, extent included: `Vector3[30]`, `int[30]`
// and `int[30][30]`. A comparison of values cannot tell a thirty-element array from
// a thirty-one-element one whose last element is never read, and the connect
// table's second extent is the difference between 900 ints and a map that
// silently reads past what was written.
TEST_CASE("GameState tranche 8 members start at their historical global defaults", "[gamestate]")
{
	GameState s;

	SECTION("no path points exist yet")
	{
		REQUIRE(s.numpathpoints == 0);
	}

	SECTION("every path point starts at the origin")
	{
		for (int i = 0; i < 30; i++) {
			INFO("path point " << i);
			REQUIRE(s.pathpoint[i].x == 0.0f);
			REQUIRE(s.pathpoint[i].y == 0.0f);
			REQUIRE(s.pathpoint[i].z == 0.0f);
		}
	}

	SECTION("no path point is connected to anything")
	{
		for (int i = 0; i < 30; i++) {
			INFO("path point " << i);
			REQUIRE(s.numpathpointconnect[i] == 0);

			for (int k = 0; k < 30; k++) {
				INFO("path point " << i << " link " << k);
				REQUIRE(s.pathpointconnect[i][k] == 0);
			}
		}
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(s.numpathpoints), int>::value);
		REQUIRE(std::is_same<decltype(s.pathpoint), Vector3[30]>::value);
		REQUIRE(std::is_same<decltype(s.numpathpointconnect), int[30]>::value);
		REQUIRE(std::is_same<decltype(s.pathpointconnect), int[30][30]>::value);
	}
}

// Moving the graph into GameState is only worth anything if two GameStates stop
// sharing it, so this is the property under test rather than the defaults above.
// The scalars it is easy to get wrong here: an array member that kept static
// storage duration, or that pointed at one shared block, would still behave
// identically for the single instance the game constructs, and the bug would only
// show up in a unit test.
//
// Every write below is seeded with a value the default does not hold, and every
// array is written at an index other than 0. Seeding with 0 would let the
// assertions pass against an array the writer never reached, and writing only
// index 0 would leave the other 29 elements - and, for the connect table, the
// other 899 - unobserved.
TEST_CASE("tranche 8 GameState members are per instance", "[gamestate]")
{
	GameState a;
	GameState b;

	SECTION("writing one instance's pathfinding graph leaves the other's at its defaults")
	{
		a.numpathpoints = 3;
		a.pathpoint[0] = Vector3(1.0f, 2.0f, 3.0f);
		a.pathpoint[2] = Vector3(-4.5f, 6.25f, 7.75f);
		a.pathpoint[29] = Vector3(8.0f, 9.0f, 10.0f);
		a.numpathpointconnect[1] = 4;
		a.numpathpointconnect[29] = 2;
		a.pathpointconnect[0][0] = 5;
		a.pathpointconnect[17][29] = 27;

		REQUIRE(b.numpathpoints == 0);

		for (int i = 0; i < 30; i++) {
			INFO("path point " << i);
			REQUIRE(b.pathpoint[i].x == 0.0f);
			REQUIRE(b.pathpoint[i].y == 0.0f);
			REQUIRE(b.pathpoint[i].z == 0.0f);
			REQUIRE(b.numpathpointconnect[i] == 0);

			for (int k = 0; k < 30; k++) {
				INFO("path point " << i << " link " << k);
				REQUIRE(b.pathpointconnect[i][k] == 0);
			}
		}
	}

	SECTION("a third instance also starts clean")
	{
		a.numpathpoints = 12;
		a.pathpoint[11] = Vector3(100.0f, 200.0f, 300.0f);
		a.numpathpointconnect[11] = 3;
		a.pathpointconnect[11][2] = 4;

		GameState c;
		REQUIRE(c.numpathpoints == 0);

		for (int i = 0; i < 30; i++) {
			INFO("path point " << i);
			REQUIRE(c.pathpoint[i].x == 0.0f);
			REQUIRE(c.pathpoint[i].y == 0.0f);
			REQUIRE(c.pathpoint[i].z == 0.0f);
			REQUIRE(c.numpathpointconnect[i] == 0);

			for (int k = 0; k < 30; k++) {
				INFO("path point " << i << " link " << k);
				REQUIRE(c.pathpointconnect[i][k] == 0);
			}
		}
	}
}

// Tranche 9 covers the two skeletal joint tables, `whichjointstartarray` and
// `whichjointendarray`. Together they describe the skeleton: row i names the two
// joints one bone spans, the one it starts from and the one it ends at. They are
// only ever meaningful as a pair - a start index with no end index alongside it
// describes nothing - so they are pinned as one group and move as one group.
//
// The values below are read from the globals themselves, while the globals are
// still the thing being read, so the literals are observed rather than assumed.
// The tranche that follows re-points these same assertions at the GameState
// members and adds the per-instance isolation the move buys.
//
// All 26 rows of both tables are read rather than a sample. The globals are
// initialised `= { 0 }`, which zeroes the whole array, so a one-element check
// would be satisfied by an initialiser that zeroed only element 0 - and this
// table is built by 52 assignments in Game::InitGame that leave nothing to fall
// back on if the declared extent and the written extent ever disagree.
//
// The declared types are pinned whole, extent included, as `int[26]`. The extent
// is the whole contract of these tables: Skeleton::DoConstraints loops to 26
// without consulting a stored count, so a 25-element table would be read out of
// bounds by a member that kept the same values and looked correct at index 0.
//
// This tranche is also where the asymmetry goes. DoConstraints received the start
// table as a parameter while reaching for the end table as a global, so the leaf
// had two read paths for one piece of data. This tranche gives both tables to
// GameState and threads both through that parameter list.
TEST_CASE("GameState tranche 9 members start at their historical global defaults", "[gamestate]")
{
	GameState s;

	SECTION("no bone starts at a joint yet")
	{
		for (int i = 0; i < 26; i++) {
			INFO("joint row " << i);
			REQUIRE(s.whichjointstartarray[i] == 0);
		}
	}

	SECTION("no bone ends at a joint yet")
	{
		for (int i = 0; i < 26; i++) {
			INFO("joint row " << i);
			REQUIRE(s.whichjointendarray[i] == 0);
		}
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(s.whichjointstartarray), int[26]>::value);
		REQUIRE(std::is_same<decltype(s.whichjointendarray), int[26]>::value);
	}
}

// Moving the tables into GameState is only worth anything if two GameStates stop
// sharing them, so this is the property under test rather than the defaults above.
// An array member that kept static storage duration, or that pointed at one shared
// block, would still behave identically for the single instance the game
// constructs, and the bug would only ever show up in a unit test.
//
// Every write below is seeded with a value the default does not hold, and every
// row is written at an index other than 0. Seeding with 0 would let the assertions
// pass against a table the writer never reached - which is precisely what would
// happen if the two members accidentally aliased one buffer that the test then
// failed to distinguish, since a defaulted aliased table reads zero from either
// name. Writing only index 0 would leave the other 25 rows of each table
// unobserved.
//
// The two tables are seeded with different values on purpose. Aliasing a member
// to a shared block would make them one table of 26, and a test that wrote the
// same number into both and compared against zero would not notice; seeding row 7
// of one to 70 and row 7 of the other to 700 makes any such sharing visible on
// whichever read checks first.
TEST_CASE("tranche 9 GameState members are per instance", "[gamestate]")
{
	GameState a;
	GameState b;

	SECTION("writing one instance's joint tables leaves the other's at its defaults")
	{
		a.whichjointstartarray[7] = 70;
		a.whichjointstartarray[0] = 71;
		a.whichjointstartarray[25] = 72;
		a.whichjointendarray[7] = 700;
		a.whichjointendarray[0] = 701;
		a.whichjointendarray[25] = 702;

		for (int i = 0; i < 26; i++) {
			INFO("joint row " << i);
			REQUIRE(b.whichjointstartarray[i] == 0);
			REQUIRE(b.whichjointendarray[i] == 0);
		}
	}

	SECTION("the two tables of one instance do not share storage")
	{
		// The pair is one bone per row, so a start index and an end index written
		// to the same slot would mean the end column overwrote the start column
		// and every bone in the skeleton collapsed onto its own far joint.
		a.whichjointstartarray[3] = 11;
		a.whichjointendarray[3] = 22;

		REQUIRE(a.whichjointstartarray[3] == 11);
		REQUIRE(a.whichjointendarray[3] == 22);

		for (int i = 0; i < 26; i++) {
			if (i == 3) {
				continue;
			}

			INFO("joint row " << i);
			REQUIRE(a.whichjointstartarray[i] == 0);
			REQUIRE(a.whichjointendarray[i] == 0);
		}
	}

	SECTION("a third instance also starts clean")
	{
		a.whichjointstartarray[17] = 5;
		a.whichjointendarray[17] = 6;

		GameState c;
		for (int i = 0; i < 26; i++) {
			INFO("joint row " << i);
			REQUIRE(c.whichjointstartarray[i] == 0);
			REQUIRE(c.whichjointendarray[i] == 0);
		}
	}
}

// Tranche 10 covers the eight globals left over in App/include/Globals.h and
// App/include/GameGlobals.h once every scalar has moved: six Vector3 and the two
// StereoMode values. `viewer` and `viewerfacing` are the camera position and the
// direction it faces, `windvector` is the wind the sprites are blown by,
// `hawkcoords` and `realhawkcoords` are the hawk's spot and where that spot ends
// up once the hawk has been swung around its own axis, `mapcenter` is the middle
// of the world map, and `stereomode` and `newstereomode` are the stereo mode in
// use and the one the options menu is building up for the next restart.
//
// Three of the eight are declared inside namespace Game, so they are written
// with that qualification here. Moving them into GameState drops the namespace
// rather than moving it, so the assertions below are the last place the bare
// name and the qualified name both have to mean the same object.
//
// The literals below are read from the globals themselves, while the globals are
// still the thing being read, so they are observed rather than assumed. The
// tranche that follows re-points these same assertions at the GameState members
// and adds the per-instance isolation the move buys.
//
// Five of the six vectors were declared with no initialiser at all, so what they
// started at was decided by Vector3's own default member initialisers rather than
// by the globals header. That is worth observing once, from the globals, rather
// than taking on trust, because the members spell the zeroing out explicitly and
// a Vector3 that dropped those initialisers would then silently change the start
// of a session.
//
// The two StereoMode values are the only pair here that shares a name: the
// enums they are declared with also spell `stereoCount` and the enumerator names,
// so those are pinned separately below. A member typed as the wrong one of the
// two stereo globals would compare equal at `stereoNone` and nothing else would
// notice.
//
// The declared types are pinned whole for the six vectors, and by value for the
// two enums, because the test projects compile with /wd4244, which suppresses the
// float-to-int conversion warning that would otherwise be the only hint that a
// member had changed from Vector3 to something narrower.
TEST_CASE("the tranche 10 globals start at the values GameState will carry", "[gamestate]")
{
	SECTION("the camera sits at the origin until a level places it")
	{
		REQUIRE(viewer.x == 0.0f);
		REQUIRE(viewer.y == 0.0f);
		REQUIRE(viewer.z == 0.0f);
	}

	SECTION("the camera faces nowhere in particular yet")
	{
		REQUIRE(viewerfacing.x == 0.0f);
		REQUIRE(viewerfacing.y == 0.0f);
		REQUIRE(viewerfacing.z == 0.0f);
	}

	SECTION("there is no wind until the level sets one")
	{
		REQUIRE(windvector.x == 0.0f);
		REQUIRE(windvector.y == 0.0f);
		REQUIRE(windvector.z == 0.0f);
	}

	SECTION("the hawk has nowhere to sit yet")
	{
		REQUIRE(Game::hawkcoords.x == 0.0f);
		REQUIRE(Game::hawkcoords.y == 0.0f);
		REQUIRE(Game::hawkcoords.z == 0.0f);
	}

	SECTION("the hawk's swung-around position starts at the origin too")
	{
		REQUIRE(Game::realhawkcoords.x == 0.0f);
		REQUIRE(Game::realhawkcoords.y == 0.0f);
		REQUIRE(Game::realhawkcoords.z == 0.0f);
	}

	SECTION("the world map has no centre until one is read")
	{
		REQUIRE(Game::mapcenter.x == 0.0f);
		REQUIRE(Game::mapcenter.y == 0.0f);
		REQUIRE(Game::mapcenter.z == 0.0f);
	}

	SECTION("stereo starts off")
	{
		REQUIRE(stereomode == stereoNone);
		REQUIRE(newstereomode == stereoNone);
		REQUIRE(stereomode == 0);
		REQUIRE(newstereomode == 0);
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(viewer), Vector3>::value);
		REQUIRE(std::is_same<decltype(viewerfacing), Vector3>::value);
		REQUIRE(std::is_same<decltype(windvector), Vector3>::value);
		REQUIRE(std::is_same<decltype(Game::hawkcoords), Vector3>::value);
		REQUIRE(std::is_same<decltype(Game::realhawkcoords), Vector3>::value);
		REQUIRE(std::is_same<decltype(Game::mapcenter), Vector3>::value);
		REQUIRE(std::is_same<decltype(stereomode), StereoMode>::value);
		REQUIRE(std::is_same<decltype(newstereomode), StereoMode>::value);
	}
}