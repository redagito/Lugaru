// Unit tests for the App layer.
//
// References AppLib and Catch2 only.
//
// Loads real levels out of Data/ and runs the data-loading surface they touch.
//
// This exists because of a family of bugs the terrain arrays had. While the
// terrain's arrays were one inline block, a write past the end of one of them
// silently landed in whatever member happened to sit next door. Splitting them
// onto the heap, which is what let a Terrain be a small stack-safe object, made
// the same write corrupt the heap instead - and loading a level was how that
// surfaced. Loading a level and then walking every patch of it turns that whole
// class of bug from a segfault into a named test failure.
//
// This is not a substitute for running the level. Nothing here uploads a texture
// or ticks physics: Setenvironment and Person::skeletonLoad both allocate GL
// resources, and no test constructs a context. What is covered is every bit of
// level data - terrain, objects, hotspots, dialogue, players - and the terrain
// surface those objects were placed onto.

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <json/json.h>

#include "Environment/Terrain.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Level/Dialog.hpp"
#include "Level/Hotspot.hpp"
#include "Objects/Object.hpp"
#include "Objects/Person.hpp"
#include "Objects/PersonType.hpp"

namespace
{

// Three levels that exercise the widest surface: the most objects, the most
// people, and the most dialogue in the shipped set.
const char* const kTestLevels[] = {
	"sventemple.json",
	"Lugaru3.json",
	"sventemple2.json",
};

// Every level loads the same heightmap, whichever environment it is set in.
const char* const kTerrainHeightmap = "Textures/HeightMap.png";

// The world scale the game runs at, set by Game::LoadStuff. Without it the
// terrain spans 256 units and no level's objects land inside it, because they
// are placed against a world 256 * 6 = 1536 units across.
const float kTerrainScale = 3 * 1 * 2;

Json::Value readMapJson(const std::string& levelFile)
{
	const std::string path = "Data/Maps/" + levelFile;
	std::ifstream file(path.c_str());
	REQUIRE(file.is_open());
	Json::Value mapData;
	file >> mapData;
	return mapData;
}

// LoadJsonLevel clears the patch table before placing a level's objects on it,
// so each level starts from empty rather than inheriting the previous one's.
void clearTerrainPatches(Terrain& terrain)
{
	for (int i = 0; i < subdivision; i++) {
		for (int j = 0; j < subdivision; j++) {
			terrain.patchobjects[i][j].clear();
		}
	}
}

// The terrain is shared between levels: it is the same heightmap every time, and
// loading it is the expensive part.
Terrain& sharedTerrain()
{
	static Terrain terrain;
	static bool loaded = false;
	if (!loaded) {
		REQUIRE(terrain.load(kTerrainHeightmap, 0, []() {}));
		terrain.CalculateNormals();
		loaded = true;
	}
	terrain.scale = kTerrainScale;
	return terrain;
}

// Everything LoadJsonLevel does that needs no GL context, in its order.
void loadLevelData(const Json::Value& map, Terrain& terrain, GameState& gamestate, GameAssets& assets)
{
	clearTerrainPatches(terrain);

	Dialog::loadDialogs(map["dialogs"]);

	if (!gamestate.stealthloading) {
		Object::LoadObjectsFromJson(map["objects"], terrain, []() {});
	}

	// The hotspot loop LoadJsonLevel runs inline, because Hotspot has no loader.
	Hotspot::hotspots.resize(map["hotspots"].size());
	for (unsigned i = 0; i < map["hotspots"].size(); i++) {
		Hotspot::hotspots[i].type = map["hotspots"][i]["type"].asInt();
		Hotspot::hotspots[i].size = map["hotspots"][i]["size"].asFloat();
		Hotspot::hotspots[i].text = map["hotspots"][i]["text"].asString();
		Hotspot::hotspots[i].position = map["hotspots"][i]["position"];
	}

	Object::ComputeCenter();
	Object::ComputeRadius();

	// Putting the objects on the terrain is what fills the patch table the
	// shadow pass then walks.
	Object::AddObjectsToTerrain(gamestate.environment, terrain, gamestate.detail);

	assets.weapons.weapons.clear();
	Person::players.clear();
	for (unsigned i = 0; i < map["players"].size(); i++) {
		unsigned id = 0;
		try {
			Person::players.push_back(std::shared_ptr<Person>(new Person(map["players"][i], 13, id, gamestate, assets)));
			id++;
		}
		catch (InvalidPersonException&) {
			INFO("level contained an invalid person, which the loader also skips");
		}
	}
}

int countPatchedObjects(const Terrain& terrain)
{
	int count = 0;
	for (int i = 0; i < subdivision; i++) {
		for (int j = 0; j < subdivision; j++) {
			count += static_cast<int>(terrain.patchobjects[i][j].size());
		}
	}
	return count;
}

} // namespace

TEST_CASE("a shipped level loads its data and walks the whole terrain surface", "[levelload]")
{
	for (const char* levelFile : kTestLevels) {
		SECTION(levelFile)
		{
			const Json::Value map = readMapJson(levelFile)["map"];
			INFO("loading " << levelFile);
			REQUIRE(map.isObject());

			Terrain& terrain = sharedTerrain();
			REQUIRE(terrain.size > 0);

			static GameState gamestate;
			static GameAssets assets;
			static bool typesLoaded = false;
			if (!typesLoaded) {
				PersonType::Load(assets.graphics);
				typesLoaded = true;
			}

			loadLevelData(map, terrain, gamestate, assets);

			REQUIRE(Dialog::dialogs.size() > 0);
			REQUIRE(Hotspot::hotspots.size() > 0);
			REQUIRE(Object::objects.size() > 0);
			REQUIRE(Person::players.size() > 0);
			// An object is registered in every patch its bounding sphere overlaps,
			// so the table holds at least one entry per object and more for the
			// wide ones. Fewer would mean the level's objects never landed on the
			// terrain at all.
			REQUIRE(countPatchedObjects(terrain) >= static_cast<int>(Object::objects.size()));

			// The patch table must contain exactly the objects that collide. An
			// object is registered in every patch its bounding sphere overlaps, so
			// it is one entry per patch, not one per object; and leaves, bushes and
			// fire are deliberately left out of it by addToTerrain.
			std::set<unsigned> collidableIds;
			for (unsigned i = 0; i < Object::objects.size(); i++) {
				const object_type type = Object::objects[i]->type;
				if (type != treeleavestype && type != bushtype && type != firetype) {
					collidableIds.insert(i);
				}
			}

			std::set<unsigned> presentIds;
			int patchedObjects = 0;
			for (int i = 0; i < subdivision; i++) {
				for (int j = 0; j < subdivision; j++) {
					for (unsigned id : terrain.patchobjects[i][j]) {
						REQUIRE(id < Object::objects.size());
						presentIds.insert(id);
						patchedObjects++;
					}
				}
			}

			// Every id names a real object, and every collidable object is
			// reachable in it. Both halves are what the unbounded patch indexing
			// in the simulation path depends on.
			REQUIRE(presentIds == collidableIds);
			REQUIRE(patchedObjects >= static_cast<int>(collidableIds.size()));

			// Every patch the draw and update code reaches. This is the step that
			// used to run off the end of the terrain's arrays.
			for (int x = 0; x < subdivision; x++) {
				for (int y = 0; y < subdivision; y++) {
					terrain.UpdateVertexArray(x, y, 1);
				}
			}

			// The shadow pass reads normals and writes colors, which are two more
			// of the arrays that outgrew their bounds.
			Light light;
			terrain.DoShadows(false, 1, light, false, []() {});

			// Then everything a frame of simulation asks the terrain for.
			for (int i = 0; i < terrain.size; i++) {
				for (int j = 0; j < terrain.size; j++) {
					const float x = static_cast<float>(i);
					const float z = static_cast<float>(j);
					(void)terrain.getHeight(x, z);
					(void)terrain.getOpacity(x, z);
					(void)terrain.getLighting(x, z);
					(void)terrain.getNormal(x, z);
				}
			}

			// The players must have arrived at real positions. Their skeletons are
			// deliberately empty: skeletonLoad is the step that builds them and it
			// uploads a texture, so it happens later in LoadJsonLevel than this
			// test reaches.
			for (const std::shared_ptr<Person>& player : Person::players) {
				REQUIRE(std::isfinite(player->coords.x));
				REQUIRE(std::isfinite(player->coords.y));
				REQUIRE(std::isfinite(player->coords.z));
			}
		}
	}
}
