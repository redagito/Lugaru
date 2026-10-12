// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// THE REPRODUCER FOR THE REMAINING MEMORY-SAFETY BUG.
//
// This test crashes with STATUS_STACK_BUFFER_OVERRUN (exit -1073740791) and it
// is deliberately left untracked and out of the suite, because a crash takes the
// whole test binary down and so cannot fail usefully - it destroys the run for
// every other test. It is kept on disk so the search does not have to start
// over.
//
// How the bug was narrowed to here:
//   - each section of SimulationTest.cpp alone: fine
//   - each pair of those sections: fine
//   - all three together: STATUS_STACK_BUFFER_OVERRUN
//   - four consecutive level loads, no simulation (LevelRepeatTest): fine
//   - three consecutive ragdoll sections with a load between each: fine
//   - three consecutive doAI sections with a load between each: THIS CRASHES
//
// So the residue is left by doAI, and it is left across level loads - it does
// not come from the load and it is not the ragdoll.
//
// The first thing to suspect, given that signature: SphereCheck
// (Person.cpp:7143) is the collision helper doAI drives, and it holds its
// working state in function-local statics - distance, olddistance,
// intersecting, firstintersecting, point, oldp1, start, end, slopethreshold -
// shared by every Person and every call. SphereCheck also re-enters RagDoll
// (Person.cpp:7198) from inside its own intersection loop, which is exactly the
// shape that clobbers that state. Nothing in that function writes
// jointstartarray, so that theory is already ruled out.

#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "Console.hpp"
#include "Game.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Objects/Person.hpp"
#include "Objects/PersonType.hpp"
#include "Environment/Terrain.hpp"
#include "Animation/Skeleton.hpp"

namespace
{

bool everyJointIsFinite(const Person& person)
{
    for (int i = 0; i < person.skeleton.joints.size(); i++) {
        const auto& position = person.skeleton.joints[i].position;
        if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) {
            return false;
        }
    }
    return true;
}

} // namespace

TEST_CASE("the AI can be repeated with a load between each", "[simulation][ragdoll]")
{
    INFO("A crash here means the AI alone leaves residue across loads.");

    static GameState gamestate;
    static GameAssets assets;
    static Console console;
    static bool typesLoaded = false;

    if (!typesLoaded) {
        PersonType::Load(assets.graphics);
        typesLoaded = true;
    }
    if (!assets.terrain) {
        assets.terrain = std::make_unique<Terrain>();
    }
    assets.terrain->scale = 3 * 1 * 2;

    gamestate.visibleloading = false;
    gamestate.gameon = 1;
    gamestate.stillloading = 0;

    // doAI takes no joint array; nothing to declare here.
    (void)0;

    // Three iterations, which is what the three-section run amounted to.
    for (int run = 0; run < 3; ++run) {
        INFO("ragdoll run " << run);

        REQUIRE(Game::LoadJsonLevel("Lugaru1", false, gamestate, assets, console));
        REQUIRE(assets.terrain->size > 0);

        Terrain& terrain = *assets.terrain;

        for (unsigned i = 0; i < Person::players.size(); i++) {
            Person::players[i]->skeleton.free = 1;
        }

        for (int frame = 0; frame < 240; frame++) {
            for (const std::shared_ptr<Person>& player : Person::players) {
                player->doAI(terrain, false, false, .01f, gamestate, assets);
            }
        }

        for (const std::shared_ptr<Person>& player : Person::players) {
            INFO("player after run " << run);
            REQUIRE(everyJointIsFinite(*player));
        }
    }
}
