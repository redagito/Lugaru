// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "GameGlobals.h"
#include "GameState.hpp"
#include "Globals.h"

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

// Tranche 6 covers sixteen more scalar globals from App/include/Globals.h and
// App/include/GameGlobals.h. These assertions read the globals themselves, which
// is the whole point: the values are pinned here while the globals are still the
// thing being read, so the literals below are observed rather than assumed. The
// tranche that follows moves each of these into a GameState member and asserts
// the same values against that member instead.
//
// Types are pinned next to the values on purpose. A value comparison such as
// `REQUIRE(x == 0)` holds just as happily for an int as for a float, so on its
// own it would let a member change type unnoticed.
TEST_CASE("the tranche 6 globals start at the values GameState will carry", "[gamestate]")
{
	SECTION("game difficulty")
	{
		REQUIRE(difficulty == 0);
	}

	SECTION("window resolution")
	{
		REQUIRE(screenwidth == 0.0f);
		REQUIRE(screenheight == 0.0f);
	}

	SECTION("view distance and fading")
	{
		REQUIRE(viewdistance == 0.0f);
		REQUIRE(fadestart == 0.0f);
	}

	SECTION("level theme")
	{
		REQUIRE(environment == 0);
	}

	SECTION("graphics detail")
	{
		REQUIRE(detail == 0);
	}

	SECTION("skin texture resolution")
	{
		REQUIRE(texdetail == 0.0f);
	}

	SECTION("blood")
	{
		REQUIRE(bloodtoggle == 0);
	}

	SECTION("camera shake")
	{
		REQUIRE(camerashake == 0.0f);
	}

	SECTION("texture filtering")
	{
		REQUIRE(trilinear == false);
	}

	SECTION("menu state")
	{
		REQUIRE(mainmenu == 0);
	}

	SECTION("main menu highlight")
	{
		REQUIRE(Game::selected == 0);
	}

	SECTION("camera orientation")
	{
		REQUIRE(Game::yaw == 0.0f);
		REQUIRE(Game::pitch == 0.0f);
	}

	SECTION("declared types are preserved from the migrated globals")
	{
		REQUIRE(std::is_same<decltype(difficulty), int>::value);
		REQUIRE(std::is_same<decltype(screenwidth), float>::value);
		REQUIRE(std::is_same<decltype(screenheight), float>::value);
		REQUIRE(std::is_same<decltype(viewdistance), float>::value);
		REQUIRE(std::is_same<decltype(fadestart), float>::value);
		REQUIRE(std::is_same<decltype(environment), int>::value);
		REQUIRE(std::is_same<decltype(detail), int>::value);
		REQUIRE(std::is_same<decltype(texdetail), float>::value);
		REQUIRE(std::is_same<decltype(bloodtoggle), int>::value);
		REQUIRE(std::is_same<decltype(camerashake), float>::value);
		REQUIRE(std::is_same<decltype(trilinear), bool>::value);
		REQUIRE(std::is_same<decltype(mainmenu), int>::value);
		REQUIRE(std::is_same<decltype(Game::selected), int>::value);
		REQUIRE(std::is_same<decltype(Game::yaw), float>::value);
		REQUIRE(std::is_same<decltype(Game::pitch), float>::value);
	}
}