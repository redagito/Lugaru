// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <vector>

#include "Devtools/ConsoleCmds.hpp"
#include "Level/Awards.hpp"

namespace
{


std::vector<std::string> commandNames()
{
	std::vector<std::string> names;
#define DECLARE_COMMAND(id) names.emplace_back(#id);
#include "Devtools/ConsoleCmds.inc"
#undef DECLARE_COMMAND
	return names;
}

} // namespace

TEST_CASE("the console command table is well formed", "[console]")
{
	const std::vector<std::string> names = commandNames();

	SECTION("the table is populated and matches the command count")
	{
		REQUIRE_FALSE(names.empty());
		REQUIRE(names.size() == static_cast<std::size_t>(cmd_count));
	}

	SECTION("command identifiers are unique")
	{
		const std::set<std::string> unique(names.begin(), names.end());
		REQUIRE(unique.size() == names.size());
	}

	SECTION("the runtime name table matches the enum size")
	{
		REQUIRE(std::size(cmd_names) == static_cast<std::size_t>(cmd_count));
		for (std::size_t i = 0; i < std::size(cmd_names); i++) {
			REQUIRE(std::string(cmd_names[i]) == names[i]);
		}
	}

	SECTION("lifecycle commands are present")
	{
		REQUIRE(std::find(names.begin(), names.end(), "quit") != names.end());
		REQUIRE(std::find(names.begin(), names.end(), "map") != names.end());
		REQUIRE(std::find(names.begin(), names.end(), "save") != names.end());
	}
}

TEST_CASE("command names are lower case identifiers", "[console]")
{
	// The parser matches names verbatim, so a stray uppercase or space would make
	// the command unreachable from the console.
	for (const std::string& name : commandNames()) {
		CAPTURE(name);
		REQUIRE_FALSE(name.empty());

		const bool allValid = std::all_of(name.begin(), name.end(), [](char c) {
			return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
		});
		REQUIRE(allValid);
	}
}
