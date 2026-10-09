// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "GameAssets.hpp"

TEST_CASE("GameAssets cannot be copied", "[assets]")
{
	// The four models are held by value and a Model owns four malloc'd buffers
	// through raw pointers, freeing them in its destructor. The copy constructor
	// GameAssets inherits by default would therefore hand two instances the same
	// four pointers, and both destructors would free them: heap corruption, from a
	// line that compiles cleanly.
	//
	// Nothing copies a GameAssets today. The only two instances are locals passed
	// by reference - Lugaru/source/main.cpp and the Person test helper - so the
	// trap is latent rather than live. Deleting the copy operations is what turns
	// the first `GameAssets b = a;` into a compile error instead.
	SECTION("copy construction is not available")
	{
		REQUIRE_FALSE(std::is_copy_constructible_v<GameAssets>);
	}

	SECTION("copy assignment is not available")
	{
		REQUIRE_FALSE(std::is_copy_assignable_v<GameAssets>);
	}

	SECTION("default construction still is")
	{
		// Guards the two assertions above against passing because GameAssets is
		// simply unusable: the fix has to forbid the copy and leave the rest alone,
		// because both live instances are default constructed.
		REQUIRE(std::is_default_constructible_v<GameAssets>);
	}
}
