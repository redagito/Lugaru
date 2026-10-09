// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <type_traits>

#include "KeyCapture.hpp"

TEST_CASE("a fresh KeyCapture has no capture in flight", "[keycapture]")
{
	// Every one of these is what the main loop assumes before the player has
	// clicked anything: it pumps events, it does not skip a row in the controls
	// menu, and there is no menu reload owed to it.
	KeyCapture capture;

	SECTION("the capture thread is not running")
	{
		REQUIRE(capture.waiting == false);
	}

	SECTION("no keybind row is armed")
	{
		// 0 is not -1: the sentinel that means "this row is not waiting for a
		// key" is -1, so 0 leaves the Forwards row nominally selected without any
		// thread behind it. Pinned because that is the value GameState carried
		// before the move, and a default of -1 here would change what the
		// controls menu draws on the first frame it is reached.
		REQUIRE(capture.keyselect == 0);
	}

	SECTION("no menu reload is pending")
	{
		// Consuming an absent request has to be harmless, because Menu::Tick asks
		// once every frame it runs and there is only a request to answer once per
		// completed rebind.
		REQUIRE(capture.takeReloadRequest() == false);
	}
}

TEST_CASE("the key capture handshake is held in atomics", "[keycapture]")
{
	// These two members are written by the capture thread and polled by the main
	// loop. As a plain bool and int that is a data race by the standard, whatever
	// the join later does for the writes that happen before it.
	//
	// std::is_atomic_v would be the obvious way to ask, but the <atomic> this
	// project builds against does not declare the trait at all, so each member is
	// named exactly instead. That is the stronger statement anyway: waiting has to
	// be a std::atomic<bool> and not merely some atomic, and keyselect an
	// std::atomic<int>, because their values are compared against false and
	// against -1 throughout Menu.cpp.
	//
	// The second assertion in each section is the property the trait would have
	// been standing in for, and the one the rest of the design rests on: an atomic
	// cannot be copied. GameState has to stay trivially copyable, so this is
	// precisely why the handshake had to leave it. These are REQUIREs rather than
	// static_asserts so that a regression is a failing test with a name and a
	// message, not a compile error pointing at this file.
	SECTION("waiting")
	{
		REQUIRE(std::is_same<decltype(KeyCapture::waiting), std::atomic<bool>>::value);
		REQUIRE_FALSE(std::is_copy_constructible_v<decltype(KeyCapture::waiting)>);
	}

	SECTION("keyselect")
	{
		REQUIRE(std::is_same<decltype(KeyCapture::keyselect), std::atomic<int>>::value);
		REQUIRE_FALSE(std::is_copy_constructible_v<decltype(KeyCapture::keyselect)>);
	}

	SECTION("reloadRequested")
	{
		REQUIRE(std::is_same<decltype(KeyCapture::reloadRequested), std::atomic<bool>>::value);
		REQUIRE_FALSE(std::is_copy_constructible_v<decltype(KeyCapture::reloadRequested)>);
	}

	SECTION("so the owner cannot be copied either")
	{
		// The consequence of the three above, and the reason the owner had to be
		// a separate object rather than two members of GameState. Default
		// construction still has to work, because main() default constructs one.
		REQUIRE_FALSE(std::is_copy_constructible_v<KeyCapture>);
		REQUIRE_FALSE(std::is_copy_assignable_v<KeyCapture>);
		REQUIRE(std::is_default_constructible_v<KeyCapture>);
	}
}

TEST_CASE("a reload request reaches the main thread exactly once", "[keycapture]")
{
	// The request is the whole of what the capture thread now asks the main
	// thread to do. If takeReloadRequest() could report the same request twice
	// the menu would be rebuilt twice from one rebind; if it swallowed the
	// request the controls menu would never show the key that was just captured.
	// Both failure modes are silent, so the transition itself is what is tested
	// rather than a count.
	KeyCapture capture;

	SECTION("nothing to take means nothing taken")
	{
		REQUIRE(capture.takeReloadRequest() == false);
	}

	SECTION("one request is taken once")
	{
		capture.reloadRequested = true;

		REQUIRE(capture.takeReloadRequest() == true);
		REQUIRE(capture.takeReloadRequest() == false);
		REQUIRE(capture.takeReloadRequest() == false);
	}

	SECTION("a second rebind is taken separately")
	{
		// The consume is a reset, not a one-shot latch: two captures in one
		// session have to be able to each ask for their own reload.
		capture.reloadRequested = true;
		REQUIRE(capture.takeReloadRequest() == true);

		capture.reloadRequested = true;
		REQUIRE(capture.takeReloadRequest() == true);
		REQUIRE(capture.takeReloadRequest() == false);
	}

	SECTION("the request survives the flag being cleared while one is pending")
	{
		// takeReloadRequest() clears the flag as it consumes it, and the thread
		// sets it before clearing waiting. A consume that raced the set must not
		// lose it, so the flag is cleared underneath a pending request and the
		// request has to still be there afterwards.
		capture.reloadRequested = true;
		capture.reloadRequested = false;
		REQUIRE(capture.takeReloadRequest() == false);

		capture.reloadRequested = true;
		REQUIRE(capture.takeReloadRequest() == true);
	}
}
