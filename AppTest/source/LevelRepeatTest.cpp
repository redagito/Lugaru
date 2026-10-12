// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// A regression test for loading the same level many times in a row, which is
// also the answer to a question asked while chasing a stack buffer overrun in
// SimulationTest.cpp.
//
// The question was whether a level load fully undoes the previous one. The
// evidence that led there: each simulation section alone does not crash, nor
// does any pair of them, but all three together crash with
// STATUS_STACK_BUFFER_OVERRUN - and Catch2 re-runs the test case body once per
// section, so "all three" meant the body ran three times.
//
// Four consecutive loads with no simulation in between do not crash. So the
// level load is not the residue; whatever the crash needs, the simulation has to
// have run between loads to leave it. That is a much smaller hypothesis than
// "LoadJsonLevel does not reset something", and it is recorded here because the
// next person to look should not re-derive it.

#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "Console.hpp"
#include "Game.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Objects/Object.hpp"
#include "Objects/Person.hpp"
#include "Objects/PersonType.hpp"
#include "Environment/Terrain.hpp"

namespace
{

bool isWithinReachOfTheWorld(const Person& person, const Terrain& terrain)
{
    const float limit = terrain.size * terrain.scale;
    return std::isfinite(person.coords.x)
        && person.coords.x > -limit && person.coords.x < limit
        && person.coords.z > -limit && person.coords.z < limit;
}

} // namespace

TEST_CASE("the same level can be loaded repeatedly", "[simulation][load]")
{
    INFO("This is a diagnostic. A crash here points at the level load.");

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

    // Four loads with nothing in between. SimulationTest needed three body runs
    // - one per section - to crash, so four guarantees this is not about how far
    // it got in any simulation.
    for (int load = 0; load < 4; ++load) {
        INFO("loading the level for the " << load << "th time");

        REQUIRE(Game::LoadJsonLevel("Lugaru1", false, gamestate, assets, console));

        // The things that must survive a load for the next one to be meaningful.
        REQUIRE(assets.terrain->size > 0);
        REQUIRE(Person::players.size() > 0);
        REQUIRE(Object::objects.size() > 0);

        for (const std::shared_ptr<Person>& player : Person::players) {
            INFO("after load " << load);
            REQUIRE(isWithinReachOfTheWorld(*player, *assets.terrain));
        }
    }
}
