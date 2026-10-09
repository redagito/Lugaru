// The console's text was the last file-scope global in Lugaru.
//
// It is std::string consoletext[15]: slot 0 is the line being typed, and slots
// 1 to 14 are scrollback. Three things were wrong with leaving it as a global.
// It had no owner, so any file with the header could read and shift it; the
// shift-and-push logic was copy-pasted into three places, which the code itself
// flagged twice with "FIXME: Reduce code duplication with GameTick (should come
// from a Console class)"; and it could not simply move into GameState, because a
// std::string is neither trivially copyable nor trivially destructible and
// GameStateTest asserts that it is both.
//
// So it gets its own owner, Console, which absorbs the two console members
// GameState was already carrying - the open flag and the cursor - so the flag,
// the cursor and the text they describe stop living in two different places.
//
// These first cases read the sources as text, because the type does not exist
// yet and a behavioural test could not compile. The behavioural cases that
// exercise push() and submit() live in the same file, added with the type.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

const char* const kAppInclude = LUGARU_APP_INCLUDE_DIR;
const char* const kAppSource = LUGARU_APP_SOURCE_DIR;

std::string readText(const char* path)
{
	std::ifstream in(path);
	if (!in) {
		return std::string();
	}
	std::ostringstream buffer;
	buffer << in.rdbuf();
	return buffer.str();
}

// Drops comments and string literals, so a word that only survives in prose
// cannot satisfy or fail an assertion.
std::string codeOnly(const std::string& text)
{
	std::string out;
	out.reserve(text.size());

	bool inLine = false;
	bool inBlock = false;
	bool inString = false;
	bool inChar = false;

	for (std::string::size_type i = 0; i < text.size(); ++i) {
		const char c = text[i];
		const char next = i + 1 < text.size() ? text[i + 1] : '\0';

		if (inLine) {
			if (c == '\n') {
				inLine = false;
				out += c;
			}
			continue;
		}
		if (inBlock) {
			if (c == '*' && next == '/') {
				inBlock = false;
				++i;
			}
			continue;
		}
		if (inString) {
			if (c == '\\') {
				++i;
			}
			else if (c == '"') {
				inString = false;
			}
			else if (c == '\n') {
				inString = false;
				out += c;
			}
			continue;
		}
		if (inChar) {
			if (c == '\\') {
				++i;
			}
			else if (c == '\'') {
				inChar = false;
			}
			continue;
		}

		if (c == '/' && next == '/') {
			inLine = true;
			++i;
			continue;
		}
		if (c == '/' && next == '*') {
			inBlock = true;
			++i;
			continue;
		}
		if (c == '"') {
			inString = true;
			continue;
		}
		if (c == '\'') {
			inChar = true;
			continue;
		}

		out += c;
	}

	return out;
}

std::vector<std::string::size_type> wholeWordPositions(const std::string& text, const std::string& word)
{
	std::vector<std::string::size_type> found;
	if (word.empty()) {
		return found;
	}

	const auto isWordChar = [](char c) {
		return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
	};

	std::string::size_type at = 0;
	while ((at = text.find(word, at)) != std::string::npos) {
		const bool leftOk = at == 0 || !isWordChar(text[at - 1]);
		const std::string::size_type after = at + word.size();
		const bool rightOk = after >= text.size() || !isWordChar(text[after]);
		if (leftOk && rightOk) {
			found.push_back(at);
		}
		at = after;
	}

	return found;
}

int countAcrossSource(const std::string& needle)
{
	int total = 0;
	for (const char* root : { kAppSource }) {
		for (const std::filesystem::directory_entry& entry :
		     std::filesystem::recursive_directory_iterator(root)) {
			if (!entry.is_regular_file()) {
				continue;
			}
			const std::string extension = entry.path().extension().string();
			if (extension != ".cpp" && extension != ".hpp" && extension != ".h") {
				continue;
			}
			const std::string code = codeOnly(readText(entry.path().string().c_str()));
			total += static_cast<int>(wholeWordPositions(code, needle).size());
		}
	}
	return total;
}

} // namespace

TEST_CASE("GameGlobals.h declares no globals at all", "[console][architecture]")
{
	// The header used to declare the console's text buffer as an extern array
	// and, for the last few tranches, was the only file left in the project
	// with a global in it. It has since been deleted outright, along with
	// Globals.h - its sibling stub from the other half of the same split.
	//
	// Be clear about what deleting it buys, because it is less than it sounds:
	// a reference to a deleted header is a hard compile error, so this pins the
	// stub against coming back under that name with those includes still
	// expected of it. It is *not* a general guarantee against new globals - a
	// fresh extern in a new file, with its own definition, would still compile
	// and link. The general guarantee is the sweeps below, which scan for the
	// shape.

	SECTION("the header was deleted, along with Globals.h")
	{
		REQUIRE_FALSE(std::filesystem::exists(std::string(kAppInclude) + "/GameGlobals.h"));
		REQUIRE_FALSE(std::filesystem::exists(std::string(kAppInclude) + "/Globals.h"));
	}

	SECTION("no surviving header spells the old console global")
	{
		// consoletext was the console's own std::string[15]: slot 0 the line
		// being typed, slots 1 to 14 the scrollback, and no guard anywhere
		// stopped a header from declaring it extern again. It is Console::line
		// and Console::history now, reached through the owner, so no header
		// under App/include may name it.
		std::vector<std::string> offenders;
		for (const std::filesystem::directory_entry& entry :
		     std::filesystem::recursive_directory_iterator(kAppInclude)) {
			if (!entry.is_regular_file()) {
				continue;
			}
			const std::string extension = entry.path().extension().string();
			if (extension != ".h" && extension != ".hpp") {
				continue;
			}
			const std::string name = entry.path().filename().string();
			if (name == "GameGlobals.h" || name == "Globals.h") {
				continue;
			}
			const std::string text = codeOnly(readText(entry.path().string().c_str()));
			if (!wholeWordPositions(text, "consoletext").empty()) {
				offenders.push_back(name);
			}
		}

		std::string names;
		for (const std::string& offender : offenders) {
			names += (names.empty() ? "" : ", ") + offender;
		}
		INFO("headers still naming consoletext: " << names);
		REQUIRE(offenders.empty());
	}
}

TEST_CASE("no source file left behind declares the console globals", "[console][architecture]")
{
	// The mirror of the header sweep above, for the definition side. A global
	// can be recreated by a stray definition in a .cpp just as easily as by an
	// extern in a header, and no compiler or linker check catches it when it
	// has internal linkage.
	SECTION("no source defines consoletext")
	{
		INFO("references to consoletext in App/source: " << countAcrossSource("consoletext"));
		REQUIRE(countAcrossSource("consoletext") == 0);
	}
}

TEST_CASE("GameState no longer carries the console flag or cursor", "[console][architecture]")
{
	const std::string path = std::string(kAppInclude) + "/GameState.hpp";
	const std::string code = codeOnly(readText(path.c_str()));

	SECTION("the header was read and still has members")
	{
		REQUIRE(code.size() > 1000);
		// Non-vacuity: the member scan below means nothing if the header went
		// empty, and this header is the one place a member could hide.
		REQUIRE(wholeWordPositions(code, "float").size() > 50);
	}

	SECTION("neither member is declared")
	{
		REQUIRE(wholeWordPositions(code, "console").empty());
		REQUIRE(wholeWordPositions(code, "consoleselected").empty());
	}
}

TEST_CASE("the console has an owner with a std::array history", "[console][architecture]")
{
	const std::string path = std::string(kAppInclude) + "/Console.hpp";
	const std::string text = readText(path.c_str());

	SECTION("the header exists and is not a stub")
	{
		INFO("Console.hpp is " << text.size() << " bytes");
		REQUIRE(std::filesystem::exists(path));
		REQUIRE(text.size() > 200);
	}

	SECTION("it uses std::array for the history")
	{
		// A std::string has a non-trivial constructor and destructor, which is
		// the case std::array exists for. A C array of them decays to a pointer
		// the moment the type is passed to a template.
		REQUIRE(text.find("std::array") != std::string::npos);
		REQUIRE(text.find("std::array<std::string") != std::string::npos);
		REQUIRE(text.find("#include <array>") != std::string::npos);
	}

	SECTION("it keeps no raw string array")
	{
		REQUIRE(text.find("std::string history[") == std::string::npos);
	}

	SECTION("it owns the shift-and-push logic")
	{
		const std::string code = codeOnly(text);
		REQUIRE(wholeWordPositions(code, "push").size() >= 1);
		REQUIRE(wholeWordPositions(code, "submit").size() >= 1);
		REQUIRE(code.find("copy_backward") != std::string::npos);
	}
}

TEST_CASE("the copy-pasted console shift is gone", "[console][architecture]")
{
	SECTION("no file shifts the history by hand any more")
	{
		// Three copies existed: GameTick.cpp and twice in ConsoleCmds.cpp, each
		// with its own reverse index loop. The count is a floor, not a target -
		// it is here so a reintroduced copy cannot pass unnoticed.
		const int loops = countAcrossSource("consoletext");
		INFO("references to consoletext in App/source: " << loops);
		REQUIRE(loops == 0);
	}

	SECTION("the FIXME that asked for this class is retired")
	{
		const int fixmes = countAcrossSource("Reduce code duplication with GameTick");
		INFO("surviving FIXMEs: " << fixmes);
		REQUIRE(fixmes == 0);
	}

	SECTION("the scan really did read the tree")
	{
		// Non-vacuity. Without this a walk that silently found nothing would
		// report zero for both counts above and read as a pass.
		REQUIRE(countAcrossSource("LoadingScreen") > 5);
	}
}

// The cases above read the sources because the type did not exist when they were
// written. These exercise it directly, which is where the shift semantics are
// actually pinned: the three copy-pasted loops all shifted the same way, and a
// rewrite that shifted forwards instead would pass every structural check above.

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