// Unit tests for the App layer.
//
// References AppLib, GameLib and Catch2 only. No GL context and no audio
// device: the GL-facing half of texture handling sits behind a seam that
// defaults to a no-op, and the audio wrappers no-op without OPENAL_Init.
//
// This is the harness the roadmap gated everything else on. Until it landed,
// every claim about level loading was compile-verified only, which is how the
// terrain-array out-of-bounds survived: the test suite passed, and loading a
// level corrupted the heap.

#include <catch2/catch_test_macros.hpp>

#include <json/json.h>

#include "Console.hpp"
#include "Game.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Objects/Person.hpp"
#include "Objects/PersonType.hpp"
#include "Utils/Folders.hpp"
#include "Utils/Log.hpp"

namespace
{

// Three maps that between them cover the shipped environments, the most
// objects, and the tutorial.
const char* const kLoadableLevels[] = {
	"sventemple",
	"Lugaru3",
	"tutorial",
};

} // namespace

TEST_CASE("a level loads without a GL context", "[levelload]")
{
	Folders::makeDirectory(Folders::getUserDataPath());
	Log::init(Folders::getUserDataPath() + "/Lugaru.log");
	REQUIRE(Folders::file_exists(Folders::getResourcePath("Textures/HeightMap.png")));

	for (const char* levelFile : kLoadableLevels) {
		SECTION(levelFile)
		{
			const std::string name = levelFile;
			INFO("loading " << name);

			static GameState gamestate;
			// Nothing is gating the loading screen here, and it draws rather than
			// loads, so keep it off.
			gamestate.visibleloading = false;
			// Nothing here can tick audio or the menu, so keep both out of it.
			gamestate.gameon = 1;
			gamestate.stillloading = 0;

			static GameAssets assets;
			static Console console;
			static bool typesLoaded = false;
			if (!typesLoaded) {
				PersonType::Load(assets.graphics);
				typesLoaded = true;
			}
			// The terrain is allocated before the context is created, which is
			// why it can be allocated here too. LoadJsonLevel dereferences it
			// immediately to clear its decals.
			if (!assets.terrain) {
				assets.terrain = std::make_unique<Terrain>();
			}

			REQUIRE(Game::LoadJsonLevel(name, false, gamestate, assets, console));

			// The world is really there, not merely parsed.
			REQUIRE(Person::players.size() > 0);
			REQUIRE(Object::objects.size() > 0);
			REQUIRE(assets.terrain->size > 0);

			// And it is tickable: a skeleton was built for every person, which
			// is the step that used to upload a skin texture through GL.
			for (const std::shared_ptr<Person>& player : Person::players) {
				REQUIRE(player->skeleton.joints.size() > 0);
				REQUIRE(player->skeleton.skinsize > 0);
				REQUIRE(std::isfinite(player->coords.x));
				REQUIRE(std::isfinite(player->coords.z));
			}
		}
	}
}
