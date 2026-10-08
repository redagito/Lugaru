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
	const std::string path = std::string(kAppInclude) + "/GameGlobals.h";
	const std::string text = readText(path.c_str());

	SECTION("the header was read")
	{
		INFO("GameGlobals.h is " << text.size() << " bytes");
		REQUIRE(text.size() > 100);
	}

	SECTION("it declares no extern")
	{
		// This is the last one. When it went, the header had nothing left to
		// declare, which is the whole point of the migration.
		const std::string code = codeOnly(text);
		INFO("extern declarations left: " << wholeWordPositions(code, "extern").size());
		REQUIRE(wholeWordPositions(code, "extern").empty());
	}

	SECTION("it was not emptied to satisfy the sweep")
	{
		// Deleting the file would satisfy the section above. These two anchors
		// mean "no globals" and "the header still exists" cannot both be true by
		// removal.
		REQUIRE(text.find("#pragma once") != std::string::npos);
		REQUIRE(text.find("namespace Game") != std::string::npos);
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
