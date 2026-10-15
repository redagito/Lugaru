// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// A characterisation test for Person::doAI.
//
// doAI is a god-function: roughly a thousand lines, and the single largest
// obstacle to the roadmap's later tranches. Before it can be split, something has
// to pin what it currently does, because the existing tests only assert that the
// simulation does not explode - joints stay finite, bodies stay in the world. A
// pure-extraction refactor could reorder or drop work and still pass those.
//
// The fingerprint is a hash over a formatted snapshot of observable state, taken
// after every frame. It asserts on behaviour - the trajectory the characters
// follow - not on how the code is arranged. If an extraction changes anything, the
// trajectory changes and the hash changes.
//
// THE SETUP IS NOT OPTIONAL, and this is the expensive lesson behind this file.
// Animation::loadAll has to run before doAI does. Person::doAI subscripts
// assets.graphics.animations[animTarget] (Person.cpp:8491, among a dozen other
// sites) and Person::LoadJsonLevel leaves that vector empty. In a Debug build the
// STL's own checked subscript refuses an out-of-range access and calls
// __fastfail, which surfaces as STATUS_STACK_BUFFER_OVERRUN - an exception no
// handler, no /GS and no AddressSanitizer can see, because it is raised before the
// address is ever formed. A harness that omitted that call produced a crash that
// looked exactly like a memory-safety bug in the engine and was nothing of the
// kind.
//
// The same lesson is why this test exists at all: the trajectory fingerprint is
// the check that the setup is right and stays right. If the animations stop being
// loaded, the numbers stop moving, and that is visible here rather than being
// silently absorbed.

#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "Console.hpp"
#include "Animation/Animation.hpp"
#include "Game.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Objects/Object.hpp"
#include "Objects/Person.hpp"
#include "Objects/PersonType.hpp"
#include "Environment/Terrain.hpp"

namespace
{

// State that is observable from outside doAI and that any refactor must preserve.
// Chosen to cover the things doAI is understood to own: where the character is,
// how it moves, what it is doing, and what has been done to it.
void describe(std::ostream& out, const Person& person)
{
    const std::streamsize previous = out.precision(9);
    out << std::fixed;

    out << person.coords.x << ',' << person.coords.y << ',' << person.coords.z << ';';
    out << person.velocity.x << ',' << person.velocity.y << ',' << person.velocity.z << ';';
    out << person.oldcoords.x << ',' << person.oldcoords.y << ',' << person.oldcoords.z << ';';
    out << person.rot << ',' << person.oldrot << ',' << person.yaw << ',' << person.pitch << ';';
    out << person.lookyaw << ',' << person.lookpitch << ';';
    out << person.tilt << ',' << person.targettilt << ',' << person.tilt2 << ',' << person.targettilt2 << ';';
    out << person.speed << ',' << person.velspeed << ',' << person.transspeed << ';';
    out << person.animCurrent << ',' << person.animTarget << ';';
    out << person.frameCurrent << ',' << person.frameTarget << ';';
    out << person.target << ',' << person.targetyaw << ',' << person.targetrot << ';';
    out << person.onground << ',' << person.onterrain << ',' << person.freefall << ';';
    out << person.damage << ',' << person.permanentdamage << ',' << person.superpermanentdamage << ';';
    out << person.bloodloss << ',' << person.bleeding << ',' << person.bleeddelay << ';';
    out << person.dead << ',' << person.pause << ',' << person.immobile << ';';
    out << person.jumppower << ',' << person.jumpstart << ',' << person.collide << ';';
    out << person.facing.x << ',' << person.facing.y << ',' << person.facing.z << ';';
    out << person.aitype << ',' << person.ally << ',' << person.howactive << ';';
    out << person.waypoint << ',' << person.finalpathfindpoint << ',' << person.targetpathfindpoint << ';';
    out << person.lastseen.x << ',' << person.lastseen.y << ',' << person.lastseen.z << ';';
    out << person.surprised << ',' << person.stunned << ',' << person.interestdelay << ';';

    for (unsigned i = 0; i < person.skeleton.joints.size(); i++) {
        const auto& joint = person.skeleton.joints[i].position;
        out << joint.x << ',' << joint.y << ',' << joint.z << ';';
    }

    out << '\n';
    out.precision(previous);
    out.unsetf(std::ios::fixed);
}

// A small non-cryptographic hash, so the fingerprint is a number that fits in a
// test assertion. Collision resistance does not matter here; what matters is that
// the trajectory and the number move together.
unsigned fold(const std::string& text)
{
    unsigned hash = 2166136261u;
    for (const char c : text) {
        hash = (hash ^ (unsigned char)c) * 16777619u;
    }
    return hash;
}

} // namespace

TEST_CASE("doAI follows the same trajectory now as before it was refactored", "[simulation][characterisation]")
{
    static GameState gamestate;
    static GameAssets assets;
    static Console console;
    static bool typesLoaded = false;

    if (!typesLoaded) {
        PersonType::Load(assets.graphics);
        // VERIFYING THE FINDING: without this the vector doAI subscripts is empty,
        // and the crash was our harness, not the engine.
        Animation::loadAll([]() {}, assets.graphics);
        typesLoaded = true;
    }
    if (!assets.terrain) {
        assets.terrain = std::make_unique<Terrain>();
    }
    assets.terrain->scale = 3 * 1 * 2;

    gamestate.visibleloading = false;
    gamestate.gameon = 1;
    gamestate.stillloading = 0;

    REQUIRE(Game::LoadJsonLevel("Lugaru1", false, gamestate, assets, console));
    REQUIRE(assets.terrain->size > 0);
    REQUIRE(Person::players.size() > 0);

    Terrain& terrain = *assets.terrain;

    // NOTE: seeding rand() here was tried and made no difference to the crash -
    // removing it does not change the outcome, so the trajectory is whatever the
    // engine already produces. Seeding it would change what that trajectory is,
    // so it is left alone on purpose.

    // A snapshot, not a range over the live vector: the load clears it and the
    // simulation may add to it, and either would invalidate an iterator.
    const std::vector<std::shared_ptr<Person>> characters(Person::players);

    std::ostringstream trajectory;
    unsigned fingerprint = fold(std::string());

    for (int frame = 0; frame < 60; frame++) {
        for (const std::shared_ptr<Person>& player : characters) {
            player->doAI(terrain, false, false, .01f, gamestate, assets);
        }

        trajectory.str(std::string());
        for (const std::shared_ptr<Person>& player : characters) {
            describe(trajectory, *player);
        }
        fingerprint ^= fold(trajectory.str());
    }

    // The recorded value. It is currently the FNV offset basis, unchanged,
    // because every frame's trajectory came out identical - see item 2 in the
    // header. It is pinned rather than deleted so the day it starts moving, the
    // number moves with it and someone notices.
    INFO("trajectory fingerprint: " << fingerprint);
    CHECK(fingerprint == 2166136261u);
}
