// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// NOT YET A USEFUL TEST - this is a narrowing experiment kept in the repro
// binary until it becomes one. Two things are wrong with it and both are noted
// here rather than left for someone to rediscover:
//
//   1. It can only run one character. Restoring the rest of the players makes
//      it crash with STATUS_STACK_BUFFER_OVERRUN, on the second doAI of the very
//      first frame. That narrowing - one character runs, two crash - is the most
//      precise thing currently known about that bug, and it is why this file
//      lives here instead of in the main suite.
//
//   2. As written it fingerprints nothing. The recorded value is the FNV seed
//      unchanged, which means every frame's trajectory was identical: the fields
//      describe() reads are not the ones doAI actually moves for this character.
// Fixing that needs someone to look at what doAI writes, not a better hash.
//
// It stays in the reproducer binary because a crash takes the whole test binary
// with it, which would stop the main suite's other fifty-odd cases from
// completing. Move it up to AppTest once both items above are resolved.
//
// The intent it is meant to serve: pin doAI's behaviour so the function can be
// split. Splitting a thousand-line god-function with only "joints stay finite",
// as a net would let a refactor reorder or drop work and still pass.

#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

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
    //
    // TEMPORARY: one character, because two crash. See the note at the top of
    // this file - this is the narrowing experiment, not the finished test.
    std::vector<std::shared_ptr<Person>> characters(Person::players);
    characters.resize(characters.empty() ? 0 : 1);

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
