// Unit tests for the Graphics layer.
//
// References GraphicsLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <string>
#include <vector>

#include "Animation/Animation.hpp"

namespace
{

bool fileExists(const char* path)
{
	FILE* file = std::fopen(path, "rb");
	if (file == nullptr) {
		return false;
	}
	std::fclose(file);
	return true;
}

} // namespace

TEST_CASE("loading an animation from disk", "[animation]")
{
	SECTION("an animation with the trailing weapontarget block loads fully")
	{
		REQUIRE(fileExists("Data/Animations/WolfIdle"));
		const Animation animation("WolfIdle", lowheight, neutral, []() {});

		REQUIRE(animation.numjoints > 0);
		REQUIRE_FALSE(animation.frames.empty());
		REQUIRE(static_cast<int>(animation.frames.size()) > 0);

		// Every frame must have one joint record per declared joint.
		for (const AnimationFrame& frame : animation.frames) {
			REQUIRE(static_cast<int>(frame.joints.size()) == animation.numjoints);
		}
	}

	SECTION("a legacy animation without that block still loads")
	{
		// Tempanim predates the weapontarget field, so the loader must treat
		// the trailing section as optional rather than running off the end of
		// the file.
		REQUIRE(fileExists("Data/Animations/Tempanim"));
		const Animation animation("Tempanim", lowheight, neutral, []() {});

		REQUIRE(animation.numjoints > 0);
		REQUIRE_FALSE(animation.frames.empty());
		for (const AnimationFrame& frame : animation.frames) {
			REQUIRE(static_cast<int>(frame.joints.size()) == animation.numjoints);
		}

		// Absent weapon targets must fall back to the zeroed default.
		for (const AnimationFrame& frame : animation.frames) {
			REQUIRE(frame.weapontarget.x == 0.0f);
			REQUIRE(frame.weapontarget.y == 0.0f);
			REQUIRE(frame.weapontarget.z == 0.0f);
		}
	}

	SECTION("the declared height and attack types are preserved")
	{
		const Animation animation("WolfIdle", highheight, normalattack, []() {});
		REQUIRE(animation.height == highheight);
		REQUIRE(animation.attack == normalattack);
	}

	SECTION("a missing animation throws")
	{
		REQUIRE_THROWS(Animation("NoSuchAnimation", lowheight, neutral, []() {}));
	}

	SECTION("the progress callback is invoked")
	{
		int calls = 0;
		REQUIRE_NOTHROW(Animation("Tempanim", lowheight, neutral, [&calls]() { calls++; }));
		REQUIRE(calls >= 1);
	}
}

TEST_CASE("animation data is internally consistent", "[animation]")
{
	// A spread of shipped animations, mixing files that carry the trailing
	// weapontarget block with those that do not.
	const char* names[] = { "WolfIdle", "Tempanim", "Dead1", "Run", "BackFlip", "Sit", "Sleep" };

	for (const char* name : names) {
		SECTION(name)
		{
			if (!fileExists((std::string("Data/Animations/") + name).c_str())) {
				SUCCEED(name << " is not present in this data set");
				return;
			}

			const Animation animation(name, lowheight, neutral, []() {});

			REQUIRE(animation.numjoints > 0);
			REQUIRE_FALSE(animation.frames.empty());

			// Frame data must be populated, not left at whatever the heap held.
			for (const AnimationFrame& frame : animation.frames) {
				REQUIRE(static_cast<int>(frame.joints.size()) == animation.numjoints);
			}
		}
	}
}

TEST_CASE("a default constructed animation is empty", "[animation]")
{
	const Animation animation;
	REQUIRE(animation.numjoints == 0);
	REQUIRE(animation.frames.empty());
	REQUIRE(animation.height == lowheight);
	REQUIRE(animation.attack == neutral);
}