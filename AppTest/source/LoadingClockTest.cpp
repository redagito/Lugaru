// Unit tests for the loading overlay's wall-clock pacer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <limits>

#include "LoadingClock.hpp"

namespace
{
	// The overlay keeps a >50ms redraw throttle, so this is the smallest slice
	// of real time one redraw can ever represent.
	constexpr float kRedrawStep = 0.05f;
}

TEST_CASE("the first advance only takes a baseline and measures no elapsed time", "[loadingclock]")
{
	LoadingClock clock;

	SECTION("however huge the first delta is, nothing is counted")
	{
		// The bug: the overlay measured its first delta against a zeroed
		// timestamp, so it counted the whole process uptime as one frame.
		REQUIRE(clock.advance(600.0f) == 0.0f);
		REQUIRE(clock.elapsed() == 0.0f);
		REQUIRE(clock.ramp() == 0.0f);
	}

	SECTION("the baseline is taken, so the next delta is counted")
	{
		clock.advance(600.0f);
		REQUIRE(clock.advance(kRedrawStep) == Catch::Approx(kRedrawStep));
		REQUIRE(clock.elapsed() == Catch::Approx(kRedrawStep));
	}

	SECTION("a fresh clock reports itself unprimed")
	{
		REQUIRE_FALSE(clock.primed());
		clock.advance(600.0f);
		REQUIRE(clock.primed());
	}
}

TEST_CASE("advancing by less than a frame does not jump the ramp", "[loadingclock]")
{
	LoadingClock clock;
	clock.advance(0.0f);

	SECTION("one sub-frame step leaves the ramp at the bottom")
	{
		REQUIRE(clock.advance(1.0f / 60.0f) == Catch::Approx(1.0f / 60.0f));
		// A frame-delta-driven ramp used to add a flat amount per call, which
		// put it well above this within the first few frames.
		REQUIRE(clock.ramp() == Catch::Approx(0.0f).margin(2.0f));
	}

	SECTION("but the sub-frame time is still counted")
	{
		clock.advance(1.0f / 60.0f);
		REQUIRE(clock.elapsed() > 0.0f);
		REQUIRE(clock.ramp() > 0.0f);
	}

	SECTION("the same wall-clock instant reached three ways agrees")
	{
		clock.advance(kRedrawStep);

		LoadingClock thirds;
		thirds.advance(0.0f);
		for (int i = 0; i < 3; i++) {
			thirds.advance(kRedrawStep / 3.0f);
		}

		REQUIRE(thirds.ramp() == Catch::Approx(clock.ramp()).margin(0.01f));
	}
}

TEST_CASE("the ramp follows elapsed time, not the number of advances", "[loadingclock]")
{
	LoadingClock oneStep;
	LoadingClock manySteps;
	oneStep.advance(0.0f);
	manySteps.advance(0.0f);

	oneStep.advance(2.0f);
	for (int i = 0; i < 40; i++) {
		manySteps.advance(kRedrawStep);
	}

	SECTION("40 throttled redraws equal one unthrottled step")
	{
		REQUIRE(manySteps.elapsed() == Catch::Approx(2.0f).margin(0.001f));
		REQUIRE(manySteps.ramp() == Catch::Approx(oneStep.ramp()).margin(0.01f));
	}

	SECTION("both arrive at the same full ramp")
	{
		REQUIRE(oneStep.ramp() == Catch::Approx(100.0f).margin(0.01f));
		REQUIRE(manySteps.ramp() == Catch::Approx(100.0f).margin(0.01f));
	}

	SECTION("a clock that is never advanced stays at the bottom")
	{
		LoadingClock idle;
		REQUIRE(idle.ramp() == 0.0f);
	}
}

TEST_CASE("a full ramp takes a fixed and visible amount of real time", "[loadingclock]")
{
	LoadingClock clock;
	clock.advance(0.0f);

	int redraws = 0;
	while (clock.ramp() < 100.0f) {
		clock.advance(kRedrawStep);
		++redraws;
		REQUIRE(redraws < 1000);
	}

	SECTION("it takes at least the intended duration")
	{
		REQUIRE(clock.elapsed() >= LoadingClock::rampSeconds);
		REQUIRE(clock.elapsed() == Catch::Approx(LoadingClock::rampSeconds).margin(kRedrawStep));
	}

	SECTION("spread over many redraws, not a handful")
	{
		REQUIRE(redraws > 20);
	}

	SECTION("a single enormous gap saturates instead of falling behind")
	{
		LoadingClock stalled;
		stalled.advance(0.0f);
		stalled.advance(3600.0f);
		REQUIRE(stalled.ramp() == 100.0f);
	}
}

TEST_CASE("the flash bleeds off in bounded time and never goes negative", "[loadingclock]")
{
	LoadingClock clock;
	clock.advance(0.0f);

	float flash = 1.0f;

	SECTION("roughly half is left after half a second")
	{
		for (int i = 0; i < 10; i++) {
			flash = LoadingClock::decayFlash(flash, clock.advance(kRedrawStep));
		}
		REQUIRE(flash == Catch::Approx(0.5f).margin(0.01f));
	}

	SECTION("a full-strength flash is essentially gone in about a second")
	{
		for (int i = 0; i < 20; i++) {
			flash = LoadingClock::decayFlash(flash, clock.advance(kRedrawStep));
		}
		REQUIRE(flash == Catch::Approx(0.0f).margin(0.01f));
	}

	SECTION("it lands on exactly zero within a bounded number of redraws")
	{
		int redraws = 0;
		while (flash > 0.0f) {
			flash = LoadingClock::decayFlash(flash, clock.advance(kRedrawStep));
			++redraws;
			REQUIRE(flash >= 0.0f);
			REQUIRE(redraws < 1000);
		}
		REQUIRE(flash == 0.0f);
		REQUIRE(static_cast<float>(redraws) * kRedrawStep <= 1.5f);
	}

	SECTION("a long stretch of throttled redraws cannot leave it pinned")
	{
		for (int i = 0; i < 100; i++) {
			flash = LoadingClock::decayFlash(flash, clock.advance(kRedrawStep));
			REQUIRE(flash >= 0.0f);
		}
		REQUIRE(flash == 0.0f);
	}

	SECTION("a single gap longer than the flash kills it outright")
	{
		REQUIRE(LoadingClock::decayFlash(0.25f, 3600.0f) == 0.0f);
	}

	SECTION("decay follows elapsed time, not the number of redraws")
	{
		LoadingClock oneShot;
		oneShot.advance(0.0f);
		for (int i = 0; i < 20; i++) {
			flash = LoadingClock::decayFlash(flash, clock.advance(kRedrawStep));
		}
		REQUIRE(LoadingClock::decayFlash(1.0f, oneShot.advance(1.0f)) == Catch::Approx(flash).margin(0.01f));
	}

	SECTION("it never grows")
	{
		REQUIRE(LoadingClock::decayFlash(0.5f, 0.0f) == 0.5f);
	}
}

TEST_CASE("a reset starts a fresh ramp without discarding the baseline", "[loadingclock]")
{
	LoadingClock clock;
	clock.advance(0.0f);
	clock.advance(1.5f);
	REQUIRE(clock.ramp() == Catch::Approx(75.0f).margin(0.01f));

	clock.reset();

	REQUIRE(clock.elapsed() == 0.0f);
	REQUIRE(clock.ramp() == 0.0f);
	// The caller owns the timestamp baseline and keeps feeding measured deltas,
	// so the next one must still be counted.
	REQUIRE(clock.primed());
	REQUIRE(clock.advance(kRedrawStep) == Catch::Approx(kRedrawStep));
}

TEST_CASE("a delta that is not a positive duration is ignored", "[loadingclock]")
{
	LoadingClock clock;
	clock.advance(0.0f);
	clock.advance(0.5f);
	REQUIRE(clock.elapsed() == Catch::Approx(0.5f));

	SECTION("zero")
	{
		REQUIRE(clock.advance(0.0f) == 0.0f);
		REQUIRE(clock.elapsed() == Catch::Approx(0.5f));
	}

	SECTION("negative")
	{
		REQUIRE(clock.advance(-1.0f) == 0.0f);
		REQUIRE(clock.elapsed() == Catch::Approx(0.5f));
	}

	SECTION("not a number")
	{
		REQUIRE(clock.advance(std::numeric_limits<float>::quiet_NaN()) == 0.0f);
		REQUIRE(clock.elapsed() == Catch::Approx(0.5f));
		REQUIRE(clock.ramp() < 100.0f);
	}
}