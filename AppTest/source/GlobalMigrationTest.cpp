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

// Every whole-word occurrence of `name` in `text`. Used so that a search cannot
// match inside a longer name: `static` must not be found in `staticassert`, and
// `tempmult` must not be found in `tempmultiplier`.
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

// The leading identifier of a line, which is how a statement `tempmult = ...`
// is told apart from a declarator such as `float tempmult = ...` that happens
// to sit on the same line as its assignment.
std::string firstWord(const std::string& code)
{
	const std::string text = trim(code);
	const std::string::size_type space = text.find_first_of(" \t(");
	return space == std::string::npos ? text : text.substr(0, space);
}

// Pulls the declared names out of everything after a declaration's type: the
// shapes that occur are "float yaw, pitch" and "int pathpointconnect[30][30]".
// Anything from the terminating semicolon on is not part of a declarator.
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
template <typename Names>
std::string join(const Names& names)
{
	std::string joined;
	for (const std::string& name : names) {
		joined += (joined.empty() ? "" : ", ");
		joined += name;
	}

	return joined;
}

// Where the renderer keeps the multiplier it borrows.
//
// DrawGLScene zeroes gamestate.multiplier for the two stretches of the frame
// that must not run on the game's time scale - the stretches around the menu
// call and around the post-swap physics - and puts the value back afterwards.
// The scratch slot it borrows into is per-call state, the same way the identical
// slot is in Weapons.cpp and Sprite.cpp. What this scan is for is the storage
// class, not the arithmetic: a slot with static storage duration is one value
// for the whole process, so a second GameState would share it and the migration
// would stop guaranteeing per-instance isolation the moment a frame is drawn.

const char* const kGameDrawSource = LUGARU_APP_SOURCE_DIR "/GameDraw.cpp";
const char* const kSaveSlotName = "tempmult";

// Blanks out everything on a line that is not code - // comments, /* */
// comments, and string and character literals - padding with spaces so that
// what is left keeps its original columns. Without the blanking, a brace or an
// equals sign inside a comment or a literal would move the brace count and the
// assignment split below.
std::string codeOnly(const std::string& line, bool& inBlockComment)
{
	std::string code;

	for (std::string::size_type i = 0; i < line.size();) {
		if (inBlockComment) {
			if (line.compare(i, 2, "*/") == 0) {
				inBlockComment = false;
				i += 2;
			}
			else {
				++i;
			}
			continue;
		}

		if (line.compare(i, 2, "//") == 0) {
			break;
		}
		if (line.compare(i, 2, "/*") == 0) {
			inBlockComment = true;
			i += 2;
			continue;
		}
		if (line[i] == '"' || line[i] == '\'') {
			const char quote = line[i];
			++i;
			while (i < line.size() && line[i] != quote) {
				i += line[i] == '\\' ? 2 : 1;
			}
			if (i < line.size()) {
				++i;
			}
			continue;
		}

		code += line[i];
		++i;
	}

	code.resize(line.size(), ' ');
	return code;
}

// The first '=' on a line that assigns rather than compares, ignoring any
// assignment nested inside brackets or parentheses, or npos if the line only
// compares. Literals are already blanked by codeOnly.
std::string::size_type topLevelAssign(const std::string& code)
{
	int brackets = 0;

	for (std::string::size_type i = 0; i < code.size(); ++i) {
		if (code[i] == '(' || code[i] == '[') {
			++brackets;
			continue;
		}
		if (code[i] == ')' || code[i] == ']') {
			--brackets;
			continue;
		}
		if (code[i] != '=' || brackets != 0) {
			continue;
		}

		const char before = i == 0 ? '\0' : code[i - 1];
		const char after = i + 1 == code.size() ? '\0' : code[i + 1];
		if (before == '=' || before == '!' || before == '<' || before == '>' ||
		    before == '+' || before == '-' || before == '*' || before == '/' ||
		    before == '%' || before == '&' || before == '|' || before == '^' ||
		    after == '=' || after == '>') {
			continue;
		}

		return i;
	}

	return std::string::npos;
}

// One appearance of the save slot, and what the surrounding code does with it.
struct SlotOccurrence
{
	int line = 0;          // 1-based, the way an editor would show it
	int depth = 0;         // enclosing braces at the point the line was reached
	bool declaration = false;
	bool write = false;
	bool read = false;
	bool hasStatic = false;
};

// Every whole-word appearance of the save slot in one source file, split into
// the three shapes it can take: the declaration, the two lines that store a
// value into it, and the two lines that take one back out.
std::vector<SlotOccurrence> findSaveSlot(const char* path, const std::string& name)
{
	std::vector<SlotOccurrence> found;

	int depth = 0;
	bool inBlockComment = false;
	int lineNumber = 0;

	for (const std::string& raw : readLines(path)) {
		++lineNumber;
		const std::string code = codeOnly(raw, inBlockComment);

		for (const std::string::size_type at : wholeWordPositions(code, name)) {
			SlotOccurrence occurrence;
			occurrence.line = lineNumber;
			occurrence.depth = depth;
			occurrence.hasStatic = !wholeWordPositions(code, "static").empty();

			const std::string::size_type assign = topLevelAssign(code);
			if (assign != std::string::npos && at > assign) {
				occurrence.read = true;
			}
			else if (firstWord(code) == name) {
				occurrence.write = true;
			}
			else {
				occurrence.declaration = true;
			}

			found.push_back(occurrence);
		}

		for (const char c : code) {
			if (c == '{') {
				++depth;
			}
			else if (c == '}') {
				--depth;
			}
		}
	}

	return found;
}

// The globals still pending migration, reviewed one by one. Every scalar here
// has a GameState member to move into; entries whose type GameState
// deliberately does not hold (Texture, Model, Text*, Terrain, Frustum, Weapons)
// are never going to leave, so this list does not have to reach zero. It does
// have to stop growing, and it has to stop holding any name that GameState
// already owns. The window handle is absent on purpose: it is not pending
// migration, it now lives in WindowContext.cpp, which WindowOwnershipTest.cpp
// checks. skybox, text, textmono and the eleven shared textures are absent for
// the same kind of reason: they are owned by GameAssets and passed by reference,
// which AssetOwnershipTest.cpp checks.
//
// What is left is consoletext, which is still data and is a std::string[15] that
// would break GameState's trivial copyability if it went in; terrain, which is
// about 2.3 MB and so cannot be a by-value member of anything on the stack; and
// weapons, which holds GL handles in a std::vector<Weapon> and would drag a heap
// allocation into every GameState.
const std::set<std::string> kPendingGlobals = {
	// App/include/Globals.h
	"terrain",
	"weapons",

	// App/include/GameGlobals.h
	"consoletext",
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
		REQUIRE(globals.size() == 3);
		REQUIRE(members.size() == 155);
	}
}

TEST_CASE("the renderer's multiplier save slot is per call, not per process", "[gamestate][migration]")
{
	// A file-scope `static` here would still behave identically for a single
	// GameState, which is why the slot could survive the migration unnoticed. What
	// it would not survive is a second instance: the slot is one value for the
	// whole process, so whichever GameState drew last would hand the other's
	// multiplier back. DrawGLScene's per-instance isolation has to hold here too,
	// and the way to hold it is that the slot has no static storage duration -
	// the same shape Weapons.cpp and Sprite.cpp already use for theirs.
	const std::vector<SlotOccurrence> slot = findSaveSlot(kGameDrawSource, kSaveSlotName);

	SECTION("the slot is declared once, inside a function, with no static storage")
	{
		std::vector<std::string> offenders;
		for (const SlotOccurrence& occurrence : slot) {
			if (!occurrence.declaration) {
				continue;
			}
			if (occurrence.depth < 1 || occurrence.hasStatic) {
				offenders.push_back("line " + std::to_string(occurrence.line) +
				                    " at brace depth " + std::to_string(occurrence.depth) +
				                    (occurrence.hasStatic ? ", static" : ""));
			}
		}

		INFO("shared save slots: " << join(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("the save and restore pairs are still balanced")
	{
		// This is the property that makes the slot safe to give a fresh lifetime
		// each call: it is only ever read back after the same call has stored into
		// it. Two stores and two takes means neither store is left dangling, so
		// nothing reads a slot that no live store filled.
		int writes = 0;
		int reads = 0;
		int declarations = 0;
		for (const SlotOccurrence& occurrence : slot) {
			writes += occurrence.write ? 1 : 0;
			reads += occurrence.read ? 1 : 0;
			declarations += occurrence.declaration ? 1 : 0;
		}

		INFO("declarations: " << declarations << ", stores: " << writes << ", takes: " << reads);
		REQUIRE(declarations == 1);
		REQUIRE(writes == 2);
		REQUIRE(reads == 2);
	}
}