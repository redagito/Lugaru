// Architecture test for the terrain parameter never being confused with the
// global terrain.
//
// `terrain` was both a global - declared extern in App/include/Globals.h,
// defined in App/source/Globals.cpp - and one of the most common parameter
// names in the engine. Thirty functions take a `Terrain&` parameter, and in
// nineteen of them the parameter is itself called `terrain`, so a bare
// `terrain` there means the PARAMETER, not the global. That makes this the one
// migration where a mechanical replacement is genuinely dangerous: turning one
// of those parameter uses into `assets.terrain->` compiles, runs, and silently
// reads a different object than the caller passed in.
//
// The project has been bitten by a mechanical sweep before - a parameter
// declared `mutliplier` matched no search for `multiplier` - so the invariant is
// asserted here rather than trusted to a reviewer.
//
// The objects cannot be built without a GL context, so this reads the sources as
// text. The test project is told where App/include, App/source and
// Lugaru/source live (LUGARU_APP_INCLUDE_DIR, LUGARU_APP_SOURCE_DIR,
// LUGARU_LUGARU_SOURCE_DIR) so the scan does not have to guess from its working
// directory.
//
// Everything is measured on code with comments and string literals blanked out,
// because a comment or a tutorial string mentioning "terrain" is neither a
// global reference nor a parameter use.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "GameAssets.hpp"

namespace
{

const char* const kGlobalsHeader = LUGARU_APP_INCLUDE_DIR "/Globals.h";
const char* const kGlobalsSource = LUGARU_APP_SOURCE_DIR "/Globals.cpp";
const char* const kGameAssetsHeader = LUGARU_APP_INCLUDE_DIR "/GameAssets.hpp";

const char* const kName = "terrain";
const char* const kOtherName = "weapons";
const char* const kTypeName = "Terrain";
const char* const kOwner = "assets";

// main.cpp is listed alongside the App tree because it is the other place that
// owns a GameAssets, and it is not under App.
const char* const kSourceDirs[] = {
	LUGARU_APP_SOURCE_DIR,
	LUGARU_LUGARU_SOURCE_DIR,
};

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

// Every whole-word occurrence of `name` in `text`, so a search cannot match
// inside a longer identifier: terrain must not be found in terraintexture, and
// weapons must not be found in num_weapons.
std::vector<std::string::size_type> wholeWordPositions(const std::string& text, const std::string& name)
{
	std::vector<std::string::size_type> found;

	for (std::string::size_type at = text.find(name); at != std::string::npos;
	     at = text.find(name, at + 1)) {
		const bool startsWord = at == 0 ||
		                        (text[at - 1] != '_' && std::isalpha(static_cast<unsigned char>(text[at - 1])) == 0);
		const std::string::size_type after = at + name.size();
		const bool endsWord = after == text.size() ||
		                      (text[after] != '_' && std::isalnum(static_cast<unsigned char>(text[after])) == 0);
		if (startsWord && endsWord) {
			found.push_back(at);
		}
	}

	return found;
}

// Blanks out everything that is not code - // comments, /* */ comments, and
// string and character literals - padding with spaces so what is left keeps every
// one of its original offsets. Without that the brace walk below would count a
// brace inside a comment, every occurrence count would include the prose, and
// every line number would drift; readCode checks the length survives.
std::string codeOnly(const std::string& text)
{
	std::string code;
	code.reserve(text.size());

	bool inBlockComment = false;
	bool inLiteral = false;
	char quote = '\0';

	std::string::size_type i = 0;
	while (i < text.size()) {
		const char c = text[i];

		if (inBlockComment) {
			if (c == '*' && i + 1 < text.size() && text[i + 1] == '/') {
				inBlockComment = false;
				code += "  ";
				i += 2;
			}
			else {
				code += (c == '\n' ? '\n' : ' ');
				++i;
			}
			continue;
		}
		if (inLiteral) {
			if (c == '\\' && i + 1 < text.size()) {
				code += "  ";
				i += 2;
				continue;
			}
			if (c == quote) {
				inLiteral = false;
			}
			code += (c == '\n' ? '\n' : ' ');
			++i;
			continue;
		}
		if (c == '/' && i + 1 < text.size() && text[i + 1] == '/') {
			// Blank to the end of the line, leaving the newline to be copied like
			// any other code character.
			while (i < text.size() && text[i] != '\n') {
				code += ' ';
				++i;
			}
			continue;
		}
		if (c == '/' && i + 1 < text.size() && text[i + 1] == '*') {
			inBlockComment = true;
			code += "  ";
			i += 2;
			continue;
		}
		if (c == '"' || c == '\'') {
			inLiteral = true;
			quote = c;
			code += ' ';
			++i;
			continue;
		}

		code += c;
		++i;
	}

	return code;
}

bool isIdentifierChar(char c)
{
	return c == '_' || std::isalnum(static_cast<unsigned char>(c)) != 0;
}

// The 1-based line an offset falls on, counting the newlines before it. The
// blanked text keeps every newline, so the count is the same one an editor would
// show.
int lineOf(const std::string& code, std::string::size_type at)
{
	return static_cast<int>(std::count(code.begin(), code.begin() + static_cast<std::ptrdiff_t>(at), '\n')) + 1;
}

// The blanked text of a file, with the one property every offset below depends on
// checked: blanking must not have consumed or added a single character. A
// drifting length would not fail any assertion here, it would just make every
// line number wrong.
std::string readCode(const char* path)
{
	const std::string text = readText(path);
	const std::string code = codeOnly(text);

	INFO("blanking " << path << " changed its length from " << text.size() << " to " << code.size());
	REQUIRE(code.size() == text.size());

	return code;
}

// One parameter list in the code that declares a `Terrain` parameter, and the
// range that parameter shadows: the list itself, plus the body behind it when
// this is a definition rather than a declaration.
struct TerrainParameter
{
	std::string::size_type listBegin = 0;
	std::string::size_type listEnd = 0;
	std::string::size_type bodyBegin = 0;
	std::string::size_type bodyEnd = 0;
	bool hasBody = false;
	std::string name;
	int line = 0;
};

// Every parameter list in `code` that declares a parameter of type `Terrain`.
//
// The walk from the closing paren to the body has to survive a
// constructor-initializer list, which is why it counts brackets rather than
// taking the next brace: `Decal::Decal(...) : position(_position) {` has a `)`
// and a `(` between the two.
std::vector<TerrainParameter> terrainParameters(const std::string& code)
{
	std::vector<TerrainParameter> found;

	for (std::string::size_type open = code.find('('); open != std::string::npos;
	     open = code.find('(', open + 1)) {
		std::string::size_type close = open;
		int depth = 0;
		for (; close < code.size(); ++close) {
			if (code[close] == '(') {
				++depth;
			}
			else if (code[close] == ')') {
				--depth;
				if (depth == 0) {
					break;
				}
			}
		}
		if (close >= code.size()) {
			continue;
		}

		const std::string inner = code.substr(open + 1, close - open - 1);
		const std::vector<std::string::size_type> types = wholeWordPositions(inner, kTypeName);
		if (types.empty()) {
			continue;
		}

		TerrainParameter parameter;
		parameter.listBegin = open;
		parameter.listEnd = close;
		parameter.line = lineOf(code, open);

		// The declared name, reached by walking forward from the type past any
		// pointers and references.
		std::string::size_type at = types.front() + std::string(kTypeName).size();
		while (at < inner.size() && std::isspace(static_cast<unsigned char>(inner[at])) != 0) {
			++at;
		}
		while (at < inner.size() && (inner[at] == '*' || inner[at] == '&')) {
			++at;
		}
		while (at < inner.size() && std::isspace(static_cast<unsigned char>(inner[at])) != 0) {
			++at;
		}
		while (at < inner.size() && isIdentifierChar(inner[at])) {
			parameter.name += inner[at];
			++at;
		}

		int brackets = 0;
		for (parameter.bodyBegin = close + 1; parameter.bodyBegin < code.size(); ++parameter.bodyBegin) {
			const char c = code[parameter.bodyBegin];
			if (c == '(' || c == '[') {
				++brackets;
			}
			else if (c == ')' || c == ']') {
				--brackets;
			}
			else if (brackets == 0) {
				if (c == '{') {
					parameter.hasBody = true;
					break;
				}
				if (c == ';') {
					break;
				}
			}
		}
		if (parameter.hasBody) {
			brackets = 0;
			for (parameter.bodyEnd = parameter.bodyBegin; parameter.bodyEnd < code.size(); ++parameter.bodyEnd) {
				const char c = code[parameter.bodyEnd];
				if (c == '{') {
					++brackets;
				}
				else if (c == '}') {
					--brackets;
					if (brackets == 0) {
						break;
					}
				}
			}
		}

		found.push_back(parameter);
	}

	return found;
}

// The offset of the name in every `<owner>.<name>` reference in `code`. Matching
// the owner whole word as well is what stops `notassets.terrain` from counting.
std::vector<std::string::size_type> ownedPositions(const std::string& code, const char* owner, const char* name)
{
	std::vector<std::string::size_type> found;

	const std::string suffix = std::string(".") + name;
	for (const std::string::size_type at : wholeWordPositions(code, owner)) {
		const std::string::size_type tail = at + std::string(owner).size();
		if (code.compare(tail, suffix.size(), suffix) == 0) {
			found.push_back(tail);
		}
	}

	return found;
}

struct Occurrence
{
	std::string file;
	int line = 0;
	std::string detail;
};

template <typename Occurrences>
std::string join(const Occurrences& occurrences)
{
	std::string joined;
	for (const Occurrence& occurrence : occurrences) {
		joined += (joined.empty() ? "" : ", ");
		joined += occurrence.file + ":" + std::to_string(occurrence.line) + " " + occurrence.detail;
	}
	return joined;
}

// Every file under `root` with one of `extensions`, sorted so a failure names
// them in a stable order.
std::vector<std::filesystem::path> readAllFiles(const char* root, const std::vector<std::string>& extensions)
{
	std::vector<std::filesystem::path> paths;
	std::error_code error;

	for (const std::filesystem::directory_entry& entry :
	     std::filesystem::recursive_directory_iterator(root, error)) {
		if (!entry.is_regular_file(error)) {
			continue;
		}
		const std::string extension = entry.path().extension().string();
		if (std::find(extensions.begin(), extensions.end(), extension) != extensions.end()) {
			paths.push_back(entry.path());
		}
	}

	if (error) {
		FAIL("could not walk " << root << ": " << error.message());
	}

	std::sort(paths.begin(), paths.end());
	return paths;
}

const std::vector<std::filesystem::path> readAllHeaders()
{
	return readAllFiles(LUGARU_APP_INCLUDE_DIR, { ".h", ".hpp" });
}

// Only the two source trees: a header declares the Terrain parameters and has no
// body behind them, so the invariant below is about definitions.
std::vector<std::filesystem::path> readAllSources()
{
	std::vector<std::filesystem::path> paths;
	for (const char* root : kSourceDirs) {
		const std::vector<std::filesystem::path> found = readAllFiles(root, { ".c", ".cpp", ".h", ".hpp" });
		paths.insert(paths.end(), found.begin(), found.end());
	}
	std::sort(paths.begin(), paths.end());
	return paths;
}

// Everything the migration could have left a global reference in: the two
// source trees plus the App headers. GameAssets.hpp is the one file that names
// terrain without an owner in front of it, because it is where the owner is
// declared, and it is checked on its own terms further down.
std::vector<std::filesystem::path> readAllScannedFiles()
{
	std::vector<std::filesystem::path> paths = readAllHeaders();
	for (const char* root : kSourceDirs) {
		const std::vector<std::filesystem::path> found = readAllFiles(root, { ".c", ".cpp", ".h", ".hpp" });
		paths.insert(paths.end(), found.begin(), found.end());
	}

	paths.erase(std::remove(paths.begin(), paths.end(), std::filesystem::path(kGameAssetsHeader)), paths.end());
	std::sort(paths.begin(), paths.end());
	return paths;
}

} // namespace

TEST_CASE("no function that takes a Terrain parameter reads assets.terrain", "[terrain][architecture]")
{
	// The collision this migration could cause. A Terrain parameter shadows the
	// global, so the global cannot be named inside one of these functions at all:
	// a bare `terrain` is the parameter, and reaching past it to
	// `assets.terrain` would compile, run, and quietly use a different object
	// than the caller passed in.
	std::vector<Occurrence> offenders;
	int found = 0;

	for (const std::filesystem::path& path : readAllSources()) {
		const std::string code = codeOnly(readText(path.string().c_str()));

		for (const TerrainParameter& parameter : terrainParameters(code)) {
			if (!parameter.hasBody) {
				continue;
			}
			++found;

			const std::string body = code.substr(parameter.bodyBegin,
			                                      parameter.bodyEnd - parameter.bodyBegin);
			if (!ownedPositions(body, kOwner, kName).empty()) {
				offenders.push_back({ path.filename().string(), parameter.line,
				                      "reads assets.terrain, parameter is " + parameter.name });
			}
		}
	}

	SECTION("the invariant holds")
	{
		INFO("functions taking a Terrain parameter that read assets.terrain: " << join(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("the scan found the functions it claims to")
	{
		// Without a pin, a parser that quietly matched nothing would leave the
		// assertion above passing for the wrong reason. Thirty is the count of
		// definitions in App/source and Lugaru/source whose parameter list
		// declares a Terrain parameter.
		INFO("definitions taking a Terrain parameter: " << found);
		REQUIRE(found == 30);
	}
}

TEST_CASE("every use of the terrain global goes through assets.terrain", "[terrain][architecture]")
{
	// The mirror of the invariant above. What is left of the old global has to be
	// reached through its new owner, so the one thing that must not survive is a
	// bare `terrain` outside a function that takes a Terrain parameter called
	// `terrain` - the only place a bare `terrain` still means the parameter
	// rather than the global.
	std::vector<Occurrence> offenders;
	int owned = 0;

	for (const std::filesystem::path& path : readAllScannedFiles()) {
		const std::string code = codeOnly(readText(path.string().c_str()));

		std::vector<std::pair<std::string::size_type, std::string::size_type>> shadowed;
		for (const TerrainParameter& parameter : terrainParameters(code)) {
			if (parameter.name != kName) {
				continue;
			}
			shadowed.emplace_back(parameter.listBegin, parameter.listEnd);
			if (parameter.hasBody) {
				shadowed.emplace_back(parameter.bodyBegin, parameter.bodyEnd);
			}
		}

		const std::vector<std::string::size_type> qualified = ownedPositions(code, kOwner, kName);
		owned += static_cast<int>(qualified.size());

		for (const std::string::size_type at : wholeWordPositions(code, kName)) {
			if (std::find(qualified.begin(), qualified.end(), at) != qualified.end()) {
				continue;
			}

			bool shadowedHere = false;
			for (const auto& range : shadowed) {
				if (range.first <= at && at <= range.second) {
					shadowedHere = true;
					break;
				}
			}
			if (shadowedHere) {
				continue;
			}

			offenders.push_back({ path.filename().string(),
			                      lineOf(code, at),
			                      "bare terrain with no Terrain parameter to shadow it" });
		}
	}

	SECTION("no bare terrain is left")
	{
		INFO("bare uses of terrain: " << join(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("the qualified uses are still there")
	{
		// The sweep above would also pass on an engine that had stopped drawing
		// the world at all, so pin that the owner is being read.
		INFO("assets.terrain references: " << owned);
		REQUIRE(owned > 100);
	}

	SECTION("the scan reached the trees it claims to")
	{
		REQUIRE(readAllScannedFiles().size() > 100);
	}
}

TEST_CASE("the globals header and source no longer name terrain or weapons", "[terrain][architecture]")
{
	// Both were externs in Globals.h and definitions in Globals.cpp. Leaving
	// either behind would keep the old global alive next to its new owner, so
	// the assertion is that neither file names them at all.
	SECTION("Globals.h names neither")
	{
		const std::string text = codeOnly(readText(kGlobalsHeader));
		INFO("terrain in Globals.h: " << wholeWordPositions(text, kName).size());
		REQUIRE(wholeWordPositions(text, kName).empty());
		REQUIRE(wholeWordPositions(text, kOtherName).empty());
	}

	SECTION("Globals.cpp names neither")
	{
		const std::string text = codeOnly(readText(kGlobalsSource));
		INFO("terrain in Globals.cpp: " << wholeWordPositions(text, kName).size());
		REQUIRE(wholeWordPositions(text, kName).empty());
		REQUIRE(wholeWordPositions(text, kOtherName).empty());
	}

	SECTION("Globals.h was not emptied to satisfy the sweep")
	{
		// Both names could be removed from an empty file, or from one that was
		// gutted while still being expected to pull in SDL for something else.
		// Anchor on the parts other translation units still depend on.
		const std::string text = readText(kGlobalsHeader);
		REQUIRE(text.find("#pragma once") != std::string::npos);
		REQUIRE(text.find("#include <SDL.h>") != std::string::npos);
		REQUIRE(text.find("#include \"Objects/Weapons.hpp\"") != std::string::npos);
	}

	SECTION("Globals.cpp was not emptied to satisfy the sweep")
	{
		const std::string text = readText(kGlobalsSource);
		REQUIRE(text.find("Copyright (C) 2003, 2010 - Wolfire Games") != std::string::npos);
		REQUIRE(text.find("#include \"Globals.h\"") != std::string::npos);
	}
}

TEST_CASE("GameAssets holds the terrain by pointer and the weapons by value", "[terrain][architecture]")
{
	// The two shapes are not interchangeable and neither is free:
	//  - `Terrain` memsets roughly 2.3 MB of fixed arrays in its constructor and
	//    GameAssets is a stack local, so it has to be behind a pointer.
	//  - `Weapons` is 24 bytes plus a heap vector, so a value costs nothing and a
	//    pointer would need an owner of its own.
	// A `Terrain terrain;` member would overflow the stack; a `Terrain*` or a
	// `Weapons*` would put back the ownership hole this migration closes, and
	// both would still compile and still run.
	REQUIRE(std::filesystem::exists(kGameAssetsHeader));

	SECTION("the header declares them as members of the owner")
	{
		// Read as text rather than through decltype, so the assertion exists
		// before the members do and fails instead of not compiling.
		const std::string text = readText(kGameAssetsHeader);
		INFO("GameAssets.hpp holds neither std::unique_ptr<Terrain> terrain; nor Weapons weapons;");
		REQUIRE_FALSE(wholeWordPositions(text, "std::unique_ptr<Terrain> terrain;").empty());
		REQUIRE_FALSE(wholeWordPositions(text, "Weapons weapons;").empty());
	}

	SECTION("it holds no Terrain by value and reaches neither through a raw pointer")
	{
		const std::string text = readText(kGameAssetsHeader);

		INFO("GameAssets.hpp still holds or points at a Terrain or a Weapons by the wrong shape");
		REQUIRE(wholeWordPositions(text, "Terrain terrain;").empty());
		REQUIRE(text.find("Terrain*") == std::string::npos);
		REQUIRE(text.find("Terrain *") == std::string::npos);
		REQUIRE(text.find("Weapons*") == std::string::npos);
		REQUIRE(text.find("Weapons *") == std::string::npos);
	}
}