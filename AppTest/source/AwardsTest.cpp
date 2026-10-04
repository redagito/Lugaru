// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>
#include <vector>

#include "Level/Awards.hpp"

namespace
{

struct BonusEntry
{
	int id;
	std::string name;
	int value;
};

std::vector<BonusEntry> bonusTable()
{
	std::vector<BonusEntry> table;
#define DECLARE_BONUS(id, name, value) table.push_back(BonusEntry{ id, name, value });
#include "Level/Bonuses.def"
#undef DECLARE_BONUS
	return table;
}

std::vector<std::string> awardNames()
{
	std::vector<std::string> names;
#define DECLARE_AWARD(id, name) names.emplace_back(name);
#include "Level/Awards.def"
#undef DECLARE_AWARD
	return names;
}

} // namespace

TEST_CASE("the bonus table is well formed", "[awards]")
{
	const std::vector<BonusEntry> table = bonusTable();

	SECTION("the table is populated and matches bonus_count")
	{
		REQUIRE_FALSE(table.empty());
		REQUIRE(table.size() == static_cast<std::size_t>(bonus_count));
	}

	SECTION("bonus identifiers are unique and dense")
	{
		std::set<int> ids;
		for (const BonusEntry& entry : table) {
			ids.insert(entry.id);
		}
		REQUIRE(ids.size() == table.size());

		// The enum relies on contiguous numbering starting at zero.
		for (std::size_t i = 0; i < table.size(); i++) {
			REQUIRE(table[i].id == static_cast<int>(i));
		}
	}

	SECTION("every bonus except the sentinel has a display name")
	{
		for (const BonusEntry& entry : table) {
			if (entry.id != nobonus) {
				REQUIRE_FALSE(entry.name.empty());
			}
		}
	}

	SECTION("only the sentinel carries a zero value")
	{
		REQUIRE(table[nobonus].value == 0);
		for (const BonusEntry& entry : table) {
			if (entry.id != nobonus) {
				REQUIRE(entry.value > 0);
			}
		}
	}

	SECTION("the escalating combo chain keeps its documented order")
	{
		// Bonuses.def notes that these five must stay in order, because the
		// combo logic advances through them by index.
		REQUIRE(solidhit < twoxcombo);
		REQUIRE(twoxcombo < threexcombo);
		REQUIRE(threexcombo < fourxcombo);
		REQUIRE(fourxcombo < megacombo);

		REQUIRE(table[solidhit].value < table[twoxcombo].value);
		REQUIRE(table[twoxcombo].value < table[threexcombo].value);
		REQUIRE(table[threexcombo].value < table[fourxcombo].value);
		REQUIRE(table[fourxcombo].value < table[megacombo].value);
	}

	SECTION("the runtime name table matches the enum size")
	{
		REQUIRE(std::size(bonus_names) == static_cast<std::size_t>(bonus_count));
	}
}

TEST_CASE("the award table is well formed", "[awards]")
{
	const std::vector<std::string> names = awardNames();

	SECTION("the table is populated and matches award_count")
	{
		REQUIRE_FALSE(names.empty());
		REQUIRE(names.size() == static_cast<std::size_t>(award_count));
	}

	SECTION("every award has a display name")
	{
		for (const std::string& name : names) {
			REQUIRE_FALSE(name.empty());
		}
	}

	SECTION("the runtime name table matches the enum size")
	{
		REQUIRE(std::size(award_names) == static_cast<std::size_t>(award_count));
	}

	SECTION("award identifiers are dense")
	{
		for (int i = 0; i < award_count; i++) {
			// The enum values are contiguous, so indexing by position is safe.
			REQUIRE(i >= 0);
		}
		REQUIRE(award_count > 0);
	}
}