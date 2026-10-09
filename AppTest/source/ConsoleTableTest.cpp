// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Devtools/ConsoleCmds.hpp"
#include "Level/Awards.hpp"

namespace
{

const char* const kDevtoolsDoc = LUGARU_REPO_ROOT "/Docs/DEVTOOLS.txt";

std::vector<std::string> commandNames()
{
	std::vector<std::string> names;
#define DECLARE_COMMAND(id) names.emplace_back(#id);
#include "Devtools/ConsoleCmds.inc"
#undef DECLARE_COMMAND
	return names;
}

std::string readText(const char* path)
{
	std::ifstream input(path);
	if (!input) {
		FAIL("could not open " << path);
	}

	std::ostringstream text;
	text << input.rdbuf();
	return text.str();
}

template <typename Strings>
std::string joinNames(const Strings& names)
{
	std::string joined;
	for (const std::string& name : names) {
		joined += (joined.empty() ? "" : ", ");
		joined += name;
	}
	return joined;
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

// A command nobody can look up is a command nobody uses, and the gap was not
// random: the four that were missing were the two that write the JSON format the
// loader prefers, plus two attribute commands nobody had mentioned.
//
// The names come from the same DECLARE_COMMAND sweep the table tests above use,
// so the list cannot drift from the code. Substring matching is deliberate: the
// file documents commands as prose, "armorreset, protectionreset" on one line,
// and pinning the exact formatting would make the guard about layout rather
// than coverage.
TEST_CASE("every console command is documented", "[console][docs]")
{
	const std::vector<std::string> names = commandNames();

	SECTION("the sweep found the whole command table")
	{
		// Without this, an include list that failed to parse or a regex that matched
		// nothing would satisfy the sweep below by finding no commands at all.
		REQUIRE(names.size() == static_cast<std::size_t>(cmd_count));
		REQUIRE(names.size() >= 40);
	}

	SECTION("the documentation is where the test expects it")
	{
		REQUIRE(std::filesystem::exists(kDevtoolsDoc));
	}

	SECTION("every command name appears in the documentation")
	{
		const std::string doc = readText(kDevtoolsDoc);

		std::vector<std::string> undocumented;
		for (const std::string& name : names) {
			if (doc.find(name) == std::string::npos) {
				undocumented.push_back(name);
			}
		}

		INFO("commands missing from Docs/DEVTOOLS.txt: " << joinNames(undocumented));
		REQUIRE(undocumented.empty());
	}
}