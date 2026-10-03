// Unit tests for the Graphics layer.
//
// References GraphicsLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <memory>
#include <string>
#include <vector>

#include "Animation/AnimationDefinitions.h"
#include "Objects/PersonType.hpp"

namespace
{

struct AnimEntry
{
	int id;
	std::string name;
	int bits;
};

std::vector<AnimEntry> animationTable()
{
	std::vector<AnimEntry> table;
#define DECLARE_ANIM(id, name, height, type, bits) table.push_back(AnimEntry{ id, name, bits });
#include "Animation/Animation.def"
#undef DECLARE_ANIM
	return table;
}

} // namespace

TEST_CASE("the animation table is well formed", "[animation]")
{
	const std::vector<AnimEntry> table = animationTable();

	SECTION("the table is populated and matches animation_count")
	{
		REQUIRE_FALSE(table.empty());
		REQUIRE(table.size() == static_cast<std::size_t>(animation_count));
	}

	SECTION("animation identifiers are unique and dense")
	{
		std::set<int> ids;
		for (const AnimEntry& entry : table) {
			ids.insert(entry.id);
		}
		REQUIRE(ids.size() == table.size());

		// The enum relies on contiguous numbering starting at zero.
		for (std::size_t i = 0; i < table.size(); i++) {
			REQUIRE(table[i].id == static_cast<int>(i));
		}
	}

	SECTION("loadable animations have names, sentinel and later entries do not")
	{
		// loadable_anim_end marks the boundary: everything before it is loaded
		// from animation files and carries a display name, everything from it
		// onwards is a marker or runtime-resolved alias and is unnamed.
		for (const AnimEntry& entry : table) {
			if (entry.id < loadable_anim_end) {
				CAPTURE(entry.id);
				REQUIRE_FALSE(entry.name.empty());
			}
			else {
				CAPTURE(entry.id);
				REQUIRE(entry.name.empty());
			}
		}
	}

	SECTION("declared bits correspond to real bits")
	{
		for (const AnimEntry& entry : table) {
			// Each entry's bit mask must only use bits below animation_bit_count.
			REQUIRE((entry.bits & ~((1 << animation_bit_count) - 1)) == 0);
		}
	}

	SECTION("the bit count is sane")
	{
		REQUIRE(animation_bit_count > 0);
		REQUIRE(animation_bit_count < 31);
	}
}

TEST_CASE("PersonType", "[persontype]")
{
	// PersonType embeds a 512*512*3 byte blood texture, so instances must be
	// heap allocated to avoid blowing the stack.
	const auto type = std::make_unique<PersonType>();

	SECTION("a default constructed type has no talk or hurt idle")
	{
		REQUIRE_FALSE(type->hasAnimTalkIdle());
		REQUIRE_FALSE(type->hasAnimHurtIdle());
	}

	SECTION("assigning a real animation enables the capability flags")
	{
		auto mutableType = std::make_unique<PersonType>();
		mutableType->animTalkIdle = wolfrunanim;
		mutableType->animHurtIdle = wolfrunanim;
		REQUIRE(mutableType->hasAnimTalkIdle());
		REQUIRE(mutableType->hasAnimHurtIdle());
	}

	SECTION("resetting back to the placeholder clears the capability flags")
	{
		auto mutableType = std::make_unique<PersonType>();
		mutableType->animTalkIdle = wolfrunanim;
		REQUIRE(mutableType->hasAnimTalkIdle());
		mutableType->animTalkIdle = tempanim;
		REQUIRE_FALSE(mutableType->hasAnimTalkIdle());
	}

	SECTION("defaults are benign")
	{
		REQUIRE(type->defaultScale == 1.f);
		REQUIRE(type->defaultDamageTolerance == 0u);
		REQUIRE(type->hasClaws == false);
		REQUIRE(type->clothes == false);
		REQUIRE(type->figureFileName.empty());
	}

	SECTION("the person_type enum covers the table size")
	{
		// PersonType::Load() resizes the table to hold both creature types.
		REQUIRE(rabbittype == 0);
		REQUIRE(wolftype == 1);
	}
}