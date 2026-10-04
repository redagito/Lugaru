// Unit tests for the App layer.
//
// References AppLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <string>
#include <vector>

namespace
{

// Paths to the three headers the migration moves names between. The test project
// is told where these live so it does not have to guess from its working
// directory, which is the Lugaru source folder because of LUGARU_TEST_WORKING_DIR.
//
// Names are compared without regard to the namespace Game wraps some globals
// in: migrating Game::selected into GameState means writing gamestate.selected,
// so the bare name is the thing that must not be declared in both places.

const char* const kGlobalsHeader = LUGARU_APP_INCLUDE_DIR "/Globals.h";
const char* const kGameGlobalsHeader = LUGARU_APP_INCLUDE_DIR "/GameGlobals.h";
const char* const kGameStateHeader = LUGARU_APP_INCLUDE_DIR "/GameState.hpp";

std::vector<std::string> readLines(const char* path)
{
	std::ifstream input(path);
	if (!input) {
		FAIL("could not open " << path);
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(input, line)) {
		lines.push_back(line);
	}
	return lines;
}

std::string trim(const std::string& text)
{
	const std::string::size_type first = text.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) {
		return std::string();
	}
	return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

bool isIdentifier(const std::string& text)
{
	if (text.empty()) {
		return false;
	}
	if (text[0] != '_' && std::isalpha(static_cast<unsigned char>(text[0])) == 0) {
		return false;
	}
	return std::all_of(text.begin(), text.end(), [](unsigned char c) {
		return c == '_' || std::isalnum(c) != 0;
	});
}

// Pulls the declared names out of everything after a declaration's type: the
// shapes that occur are "float yaw, pitch", "int pathpointconnect[30][30]" and
// "SDL_Window* sdlwindow". Anything from the terminating semicolon on is not
// part of a declarator.
std::vector<std::string> declaredNames(const std::string& full_declaration)
{
	std::vector<std::string> names;

	const std::string::size_type semicolon = full_declaration.find(';');
	const std::string declaration = full_declaration.substr(0, semicolon);

	for (const std::string& raw : [&]() {
		std::vector<std::string> parts;
		std::string::size_type start = 0;
		while (start <= declaration.size()) {
			const std::string::size_type comma = declaration.find(',', start);
			parts.push_back(trim(declaration.substr(start, comma - start)));
			start = comma == std::string::npos ? declaration.size() + 1 : comma + 1;
		}
		return parts;
	}()) {
		std::string declarator = raw;

		const std::string::size_type bracket = declarator.find('[');
		if (bracket != std::string::npos) {
			declarator = trim(declarator.substr(0, bracket));
		}

		const std::string::size_type space = declarator.find_last_of(" \t");
		std::string name = trim(declarator.substr(space == std::string::npos ? 0 : space + 1));
		while (!name.empty() && (name.front() == '*' || name.front() == '&')) {
			name.erase(name.begin());
		}
		while (!name.empty() && (name.back() == '*' || name.back() == '&')) {
			name.pop_back();
		}

		if (isIdentifier(name)) {
			names.push_back(name);
		}
	}

	return names;
}

std::string withoutComment(const std::string& line)
{
	const std::string::size_type marker = line.find("//");
	return marker == std::string::npos ? line : line.substr(0, marker);
}

// Every name declared extern by one of the two globals headers.
std::set<std::string> readDeclaredGlobals(const char* path)
{
	std::set<std::string> names;

	for (const std::string& line : readLines(path)) {
		const std::string text = trim(withoutComment(line));
		if (!text.starts_with("extern ")) {
			continue;
		}
		const std::vector<std::string> declared = declaredNames(text.substr(7));
		names.insert(declared.begin(), declared.end());
	}

	return names;
}

std::set<std::string> declaredGlobals()
{
	std::set<std::string> names = readDeclaredGlobals(kGlobalsHeader);
	const std::set<std::string> inNamespace = readDeclaredGlobals(kGameGlobalsHeader);
	names.insert(inNamespace.begin(), inNamespace.end());
	return names;
}

// Every member of the GameState struct, read from its header so that a member
// added by a later tranche joins the check without this file being told.
std::set<std::string> gameStateMembers()
{
	std::set<std::string> names;

	for (const std::string& line : readLines(kGameStateHeader)) {
		const std::string text = trim(withoutComment(line));
		if (text.empty() || text.front() == '#') {
			continue;
		}

		const std::string::size_type semicolon = text.find(';');
		if (semicolon == std::string::npos) {
			continue;
		}

		const std::string::size_type assignment = text.find('=');
		const std::string declaration = trim(text.substr(0, assignment == std::string::npos ? semicolon : assignment));
		const std::vector<std::string> declared = declaredNames(declaration);
		names.insert(declared.begin(), declared.end());
	}

	return names;
}

std::string join(const std::set<std::string>& names)
{
	std::string joined;
	for (const std::string& name : names) {
		joined += (joined.empty() ? "" : ", ");
		joined += name;
	}
	return joined;
}

// The globals still pending migration, reviewed one by one. Every scalar here
// has a GameState member to move into; entries whose type GameState
// deliberately does not hold (Texture, Model, Text*, Terrain, Frustum, Weapons,
// an SDL_Window*) are never going to leave, so this list does not have to reach
// zero. It does have to stop growing, and it has to stop holding any name that
// GameState already owns.
const std::set<std::string> kPendingGlobals = {
	// App/include/Globals.h
	"multiplier",
	"viewer", "viewerfacing",
	"light", "terrain",
	"sdlwindow", "frustum", "weapons",
	"windvector", "whichjointstartarray",
	"whichjointendarray", "stereomode",
	"newstereomode",

	// App/include/GameGlobals.h
	"terraintexture", "terraintexture2", "loadscreentexture", "Mapcircletexture",
	"Maparrowtexture", "Mapboxtexture", "cursortexture", "screentexture",
	"screentexture2", "Mainmenuitems", "skybox",
	"hawk", "hawktexture", "hawkcoords", "realhawkcoords", "eye",
	"cornea", "iris", "mapcenter", "text",
	"textmono", "pathpoint", "numpathpoints",
	"numpathpointconnect", "pathpointconnect", "consoletext",
};

} // namespace

TEST_CASE("the globals headers only lose globals to GameState", "[gamestate][migration]")
{
	const std::set<std::string> globals = declaredGlobals();
	const std::set<std::string> members = gameStateMembers();

	SECTION("nothing GameState owns is declared as a global as well")
	{
		// Putting a migrated name back would give the engine two values to keep
		// in step, and only one of them is the one GameState carries around.
		std::set<std::string> reintroduced;
		std::set_intersection(globals.begin(), globals.end(), members.begin(), members.end(),
			std::inserter(reintroduced, reintroduced.begin()));

		INFO("redeclared by the globals headers: " << join(reintroduced));
		REQUIRE(reintroduced.empty());
	}

	SECTION("the globals still pending are exactly the reviewed ones")
	{
		// A name leaving this list is migration progress. A name arriving is a
		// new global that has to be looked at before it becomes the norm.
		std::set<std::string> unreviewed;
		std::set_difference(globals.begin(), globals.end(), kPendingGlobals.begin(), kPendingGlobals.end(),
			std::inserter(unreviewed, unreviewed.begin()));

		std::set<std::string> noLongerDeclared;
		std::set_difference(kPendingGlobals.begin(), kPendingGlobals.end(), globals.begin(), globals.end(),
			std::inserter(noLongerDeclared, noLongerDeclared.begin()));

		INFO("declared but not pending: " << join(unreviewed));
		INFO("pending but no longer declared: " << join(noLongerDeclared));
		REQUIRE(unreviewed.empty());
		REQUIRE(noLongerDeclared.empty());
	}

	SECTION("the parse found what the headers actually say")
	{
		// Without this, a parser that quietly matched nothing would leave every
		// assertion above passing for the wrong reason.
		REQUIRE(globals.size() == 39);
		REQUIRE(members.size() == 138);
	}
}