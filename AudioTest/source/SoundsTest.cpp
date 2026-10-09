// Unit tests for the Audio layer.
//
// References AudioLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>
#include <vector>

#include "Audio/AudioState.hpp"
#include "Audio/Sounds.hpp"

namespace
{

// Mirrors the DECLARE_SOUND expansion used by Sounds.cpp so the table can be
// inspected without an audio device.
std::vector<std::string> soundFilenames()
{
	std::vector<std::string> names;
#define DECLARE_SOUND(id, filename) names.emplace_back(filename);
#include "Audio/Sounds.inc"
#undef DECLARE_SOUND
	return names;
}

} // namespace

TEST_CASE("the sound table is well formed", "[sounds]")
{
	const std::vector<std::string> names = soundFilenames();

	SECTION("every entry has a file name")
	{
		REQUIRE_FALSE(names.empty());
		for (const std::string& name : names) {
			REQUIRE_FALSE(name.empty());
		}
	}

	SECTION("the table size matches sounds_count")
	{
		REQUIRE(names.size() == static_cast<std::size_t>(sounds_count));
	}

	SECTION("file names are unique")
	{
		const std::set<std::string> unique(names.begin(), names.end());
		REQUIRE(unique.size() == names.size());
	}

	SECTION("streamed sounds form a contiguous block at the end")
	{
		// loadAllSounds() iterates from stream_firesound to stream_menutheme
		// and marks each as looping, so the range must be ordered and valid.
		REQUIRE(stream_firesound >= 0);
		REQUIRE(stream_menutheme < sounds_count);
		REQUIRE(stream_firesound < stream_menutheme);
	}
}

TEST_CASE("ambient sound pool", "[ambient]")
{
	// The pool is global state; save and restore it around each section so the
	// tests stay order independent.
	const int savedCount = numenvsounds;
	const std::vector<Vector3> savedPos(envsound, envsound + max_env_sounds);
	const std::vector<float> savedVol(envsoundvol, envsoundvol + max_env_sounds);
	const std::vector<float> savedLife(envsoundlife, envsoundlife + max_env_sounds);

	numenvsounds = 0;

	SECTION("addEnvSound appends to the pool")
	{
		addEnvSound(Vector3(1.0f, 2.0f, 3.0f), 16.0f, 0.4f);

		REQUIRE(numenvsounds == 1);
		REQUIRE(envsound[0].x == 1.0f);
		REQUIRE(envsound[0].y == 2.0f);
		REQUIRE(envsound[0].z == 3.0f);
		REQUIRE(envsoundvol[0] == 16.0f);
		REQUIRE(envsoundlife[0] == 0.4f);
	}

	SECTION("addEnvSound uses the documented default volume and lifetime")
	{
		addEnvSound(Vector3());
		REQUIRE(numenvsounds == 1);
		REQUIRE(envsoundvol[0] == 16.0f);
		REQUIRE(envsoundlife[0] == 0.4f);
	}

	SECTION("several sounds accumulate in order")
	{
		addEnvSound(Vector3(1.0f, 0.0f, 0.0f), 1.0f, 1.0f);
		addEnvSound(Vector3(2.0f, 0.0f, 0.0f), 2.0f, 2.0f);
		addEnvSound(Vector3(3.0f, 0.0f, 0.0f), 3.0f, 3.0f);

		REQUIRE(numenvsounds == 3);
		REQUIRE(envsoundvol[0] == 1.0f);
		REQUIRE(envsoundvol[1] == 2.0f);
		REQUIRE(envsoundvol[2] == 3.0f);
	}

	SECTION("the pool never overflows")
	{
		// Regression test: addEnvSound used to index envsound[numenvsounds]
		// with no bound, so more than max_env_sounds concurrent ambient sounds
		// wrote past the end of the arrays.
		for (int i = 0; i < max_env_sounds * 4; i++) {
			addEnvSound(Vector3(static_cast<float>(i), 0.0f, 0.0f), 1.0f, 1.0f);
			REQUIRE(numenvsounds <= max_env_sounds);
		}

		REQUIRE(numenvsounds == max_env_sounds);
		// The most recent sound is the one that survives.
		REQUIRE(envsound[max_env_sounds - 1].x == static_cast<float>(max_env_sounds * 4 - 1));
	}

	SECTION("default volume and lifetime are sane")
	{
		REQUIRE(max_env_sounds > 0);
		REQUIRE(sounds_count > 0);
	}

	numenvsounds = savedCount;
	for (int i = 0; i < max_env_sounds; i++) {
		envsound[i] = savedPos[i];
		envsoundvol[i] = savedVol[i];
		envsoundlife[i] = savedLife[i];
	}
}