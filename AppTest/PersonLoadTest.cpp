// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include "Objects/Person.hpp"

TEST_CASE("Person weapon limits", "[person]")
{
	// Regression test: weaponids[] holds PersonLimits::max_weapons entries, but
	// the loader used to accept 5 and write weaponids[4] past the end.
	SECTION("the limit matches the array size")
	{
		REQUIRE(PersonLimits::max_weapons == 4);
	}

	SECTION("counts within the limit are accepted")
	{
		REQUIRE(PersonLimits::weaponCountIsValid(0));
		REQUIRE(PersonLimits::weaponCountIsValid(1));
		REQUIRE(PersonLimits::weaponCountIsValid(2));
		REQUIRE(PersonLimits::weaponCountIsValid(3));
		REQUIRE(PersonLimits::weaponCountIsValid(PersonLimits::max_weapons));
	}

	SECTION("counts past the limit are rejected")
	{
		REQUIRE_FALSE(PersonLimits::weaponCountIsValid(PersonLimits::max_weapons + 1));
		REQUIRE_FALSE(PersonLimits::weaponCountIsValid(5));
		REQUIRE_FALSE(PersonLimits::weaponCountIsValid(6));
		REQUIRE_FALSE(PersonLimits::weaponCountIsValid(64));
	}

	SECTION("negative counts are rejected")
	{
		REQUIRE_FALSE(PersonLimits::weaponCountIsValid(-1));
		REQUIRE_FALSE(PersonLimits::weaponCountIsValid(-100));
	}
}

TEST_CASE("Person waypoint limits", "[person]")
{
	// Regression test: waypoints[]/waypointtype[] hold max_waypoints entries, but
	// the loaders used to iterate over whatever count the file declared.
	SECTION("the limit matches the array size")
	{
		REQUIRE(PersonLimits::max_waypoints == 90);
	}

	SECTION("counts within the limit are accepted")
	{
		REQUIRE(PersonLimits::waypointCountIsValid(0));
		REQUIRE(PersonLimits::waypointCountIsValid(1));
		REQUIRE(PersonLimits::waypointCountIsValid(29));
		REQUIRE(PersonLimits::waypointCountIsValid(30));
		REQUIRE(PersonLimits::waypointCountIsValid(PersonLimits::max_waypoints));
	}

	SECTION("counts past the limit are rejected")
	{
		REQUIRE_FALSE(PersonLimits::waypointCountIsValid(PersonLimits::max_waypoints + 1));
		REQUIRE_FALSE(PersonLimits::waypointCountIsValid(1000));
		REQUIRE_FALSE(PersonLimits::waypointCountIsValid(10000));
	}

	SECTION("negative counts are rejected")
	{
		REQUIRE_FALSE(PersonLimits::waypointCountIsValid(-1));
		REQUIRE_FALSE(PersonLimits::waypointCountIsValid(-100));
	}
}