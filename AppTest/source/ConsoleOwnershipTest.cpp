// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "Console.hpp"


TEST_CASE("a fresh console is closed and empty", "[console]")
{
	const Console console;

	REQUIRE_FALSE(console.open);
	REQUIRE(console.selected == 0);
	REQUIRE(console.line.empty());
	REQUIRE(console.history.size() == 14);
	for (const std::string& entry : console.history) {
		REQUIRE(entry.empty());
	}
}

TEST_CASE("push makes its text the current line and shifts the old one back", "[console]")
{
	Console console;

	SECTION("the first push has nothing to shift")
	{
		console.push("first");
		REQUIRE(console.line == "first");
		REQUIRE(console.history[0].empty());
	}

	SECTION("the second push pushes the first into the history")
	{
		console.push("first");
		console.push("second");
		REQUIRE(console.line == "second");
		REQUIRE(console.history[0] == "first");
		REQUIRE(console.history[1].empty());
	}

	SECTION("order is newest first, oldest last")
	{
		console.push("one");
		console.push("two");
		console.push("three");
		REQUIRE(console.line == "three");
		REQUIRE(console.history[0] == "two");
		REQUIRE(console.history[1] == "one");
		REQUIRE(console.history[2].empty());
	}

	SECTION("the cursor goes back to the start")
	{
		console.selected = 6;
		console.push("x");
		REQUIRE(console.selected == 0);
	}
}

TEST_CASE("submit moves the line into the history and clears it", "[console]")
{
	Console console;

	console.push("earlier");
	console.line = "map map1";
	console.submit();

	REQUIRE(console.line.empty());
	REQUIRE(console.history[0] == "map map1");
	// "earlier" was only ever the console's own message: it sat in the line
	// until the typed text displaced it, so it never reached the scrollback.
	REQUIRE(console.history[1].empty());
	REQUIRE(console.selected == 0);
}

TEST_CASE("the history never grows past fourteen entries", "[console]")
{
	Console console;

	// One more than it can hold, so the oldest has to fall off the back. This is
	// the bound the old magic 14 enforced by hand in three places.
	for (int i = 0; i < 15; i++) {
		console.push("line " + std::to_string(i));
	}

	REQUIRE(console.history.size() == 14);
	REQUIRE(console.line == "line 14");
	REQUIRE(console.history[0] == "line 13");
	// The first push of the loop had nothing in the line to record, so the
	// oldest entry that survives is the second one, not the first.
	REQUIRE(console.history[13] == "line 0");
}

TEST_CASE("submitting an empty line still shifts nothing into the history", "[console]")
{
	Console console;

	console.push("only");
	console.line.clear();
	console.submit();

	REQUIRE(console.line.empty());
	REQUIRE(console.history[0].empty());
	REQUIRE(console.history[1].empty());
}
