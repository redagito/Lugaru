// Architecture test for the editor's path point selection sentinel.
//
// `gamestate.pathpointselected` holds -1 as a valid "nothing is selected" value.
// GameTick.cpp runs it off the end of the list on `.` and back on `,`, and both
// the delete command and the second half of the connect command test it against
// -1 before subscripting the graph with it. So -1 is not a transient state - it
// is the state the editor rests in once the player has tabbed past the last
// point - and a site that subscripts the graph with it unchecked reads or writes
// before the start of the array.
//
// GameDraw.cpp was the first site found. Its pathfind link block was guarded by
// `numpathpoints > 1` alone, and the marker point at the end of that block read
// `pathpoint[pathpointselected]`, so with two or more waypoints and none
// selected it read pathpoint[-1] once per frame.
//
// GameTick.cpp:1659-1667 is the second, and the worse of the two: the first half
// of the connect command increments `numpathpointconnect[pathpointselected]`
// inside the same `numpathpoints > 1` guard, and `i != pathpointselected` in its
// body is trivially true when nothing is selected, so it cannot stand in for the
// check. Both now guard with `numpathpoints > 1 && pathpointselected != -1` -
// GameTick.cpp already used exactly that idiom further down the same keybind, at
// `:1676`.
//
// Neither can be executed by a unit test - the drawing needs a GL context and the
// editor commands need a key event - so the assertions below read the source as
// text. They ask the question rather than matching a line: for every subscript
// whose index expression names the selection, is some enclosing `if` condition
// testing `pathpointselected != -1`? Following the enclosing conditions by
// brace depth instead of by adjacency is what makes this an invariant - it does
// not care which line the guard sits on or how it is spaced, and it keeps
// holding if a site is moved, given another enclosing block, or joined by a
// second one.
//
// The scope is every .c, .cpp, .h and .hpp under App/include and App/source: 15
// subscripts over two files today, rather than the one file the assertion began
// with. A file that starts indexing the graph is then covered without having to
// be named here.
//
// The test project is told where those two trees live (LUGARU_APP_INCLUDE_DIR
// and LUGARU_APP_SOURCE_DIR) so the scan does not have to guess from its
// working directory.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace
{

// Both trees App is built from, walked whole rather than reduced to the two
// files that happen to index the graph today, so that a subscript added to a
// file this test has never heard of is still covered. Both locations are handed
// over by AppTest/CMakeLists.txt, so neither has to be guessed from the working
// directory.
const char* const kAppTrees[] = {
	LUGARU_APP_INCLUDE_DIR,
	LUGARU_APP_SOURCE_DIR,
};

const std::vector<std::string> kSourceExtensions = { ".c", ".cpp", ".h", ".hpp" };

// The file the marker point is drawn in. Named only so that the count below can
// be pinned to it: the sweep itself does not use it.
const char* const kMarkerFile = "GameDraw.cpp";

// The three members an index expression can be applied to. Whole-word matching
// is what keeps them apart: `pathpoint` is a prefix of both `pathpointconnect`
// and `pathpointselected`, and `pathpointconnect` is a suffix of
// `numpathpointconnect`, which is indexed differently.
const char* const kGraphArrays[] = {
	"pathpoint",
	"pathpointconnect",
	"numpathpointconnect",
};

// The selection, and the one shape of the check that rules out its -1.
const char* const kSelection = "pathpointselected";
const char* const kNoSelectionCheck = "pathpointselected!=-1";

// How many subscripts the tree is expected to make with the selection: three in
// GameDraw.cpp, and the twelve in GameTick.cpp that the connect command, the add
// half of the connect command and the delete command make. Pinned so that a
// deleted or renamed site cannot leave the assertions below holding over
// nothing.
const int kExpectedSites = 15;

// How many of them belong to the marker point alone: the x, the y and the z of
// the single glVertex3f GameDraw.cpp draws for the selected path point. Pinned
// apart from the total so that dropping the marker point cannot pass as a fix
// for the out-of-bounds read - it would take this count with it.
const int kExpectedMarkerSites = 3;

// Every source file under the two trees, sorted, so that a failure names them in
// a stable order.
std::vector<std::filesystem::path> readAllAppSources()
{
	std::vector<std::filesystem::path> paths;
	std::error_code error;

	for (const char* const root : kAppTrees) {
		for (const std::filesystem::directory_entry& entry :
		     std::filesystem::recursive_directory_iterator(root, error)) {
			if (!entry.is_regular_file(error)) {
				continue;
			}
			const std::string extension = entry.path().extension().string();
			if (std::find(kSourceExtensions.begin(), kSourceExtensions.end(), extension) !=
			    kSourceExtensions.end()) {
				paths.push_back(entry.path());
			}
		}
	}

	if (error) {
		FAIL("could not walk the App trees: " << error.message());
	}

	std::sort(paths.begin(), paths.end());
	return paths;
}

// A path as a failure message should name it: relative to the tree it was found
// under, so the message reads `Objects/Person.cpp:42` instead of printing the
// whole configured path.
std::string displayPath(const std::filesystem::path& path)
{
	for (const char* const root : kAppTrees) {
		const std::filesystem::path relative = path.lexically_relative(root);
		if (!relative.empty() && *relative.begin() != "..") {
			return relative.generic_string();
		}
	}

	return path.filename().string();
}

std::vector<std::string> readLines(const std::filesystem::path& path)
{
	std::ifstream input(path.string().c_str());
	if (!input) {
		FAIL("could not open " << path.string());
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

// Every whole-word occurrence of `name` in `text`, so a search cannot match
// inside a longer identifier.
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

// Blanks out everything on a line that is not code - // comments, /* */
// comments, and string and character literals - padding with spaces so that
// what is left keeps its original columns. Without the blanking, a brace inside
// a comment would shift the depth the whole rest of the file is measured
// against, and a `pathpoint[` written in prose would read as a subscript.
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

int parenBalance(const std::string& text)
{
	int balance = 0;
	for (const char c : text) {
		if (c == '(') {
			++balance;
		}
		else if (c == ')') {
			--balance;
		}
	}
	return balance;
}

// The condition that opened the brace at `brace`, which is taken to be the
// parenthesised expression immediately in front of it. Read backwards from the
// brace so nothing has to be remembered from the `if` to its brace, and so a
// condition wrapped over several lines is picked up whole by prepending lines
// until the parentheses balance.
//
// A brace that is not opened by a parenthesised condition - a function body, a
// namespace, a plain block - yields an empty string, which is a scope that
// guards nothing. It is not an error: those blocks really do run
// unconditionally.
std::string conditionBeforeBrace(const std::vector<std::string>& lines, std::string::size_type line, std::string::size_type brace)
{
	std::string::size_type end = brace;
	while (end > 0 && std::isspace(static_cast<unsigned char>(lines[line][end - 1])) != 0) {
		--end;
	}
	if (end == 0 || lines[line][end - 1] != ')') {
		return std::string();
	}

	std::string condition;
	for (std::string::size_type l = line;; --l) {
		const std::string head = trim(l == line ? lines[l].substr(0, end) : lines[l]);
		condition = condition.empty() ? head : head + " " + condition;
		if (parenBalance(condition) >= 0 || l == 0) {
			break;
		}
	}

	return condition;
}

// What is inside one subscript: `pathpoint[gamestate.pathpointselected]` gives
// `gamestate.pathpointselected`, and `pathpoint[pathpointconnect[k][i]]` gives
// the inner subscript whole, because the brackets are balanced outside in. A
// subscript that does not close on its own line yields an empty string, so a
// site is never counted on half an expression.
std::string subscriptExpression(const std::string& code, std::string::size_type open)
{
	int depth = 0;
	for (std::string::size_type i = open; i < code.size(); ++i) {
		if (code[i] == '[') {
			++depth;
		}
		else if (code[i] == ']') {
			--depth;
			if (depth == 0) {
				return code.substr(open + 1, i - open - 1);
			}
		}
	}

	return std::string();
}

// Whether a guard condition rules out the sentinel. Whitespace is squashed
// first, so every spacing of the one idiom the codebase uses reads the same:
// `!= -1`, `!=-1`, and any of them inside parentheses. `pathpoint] != -1` does
// not match, because the `]` survives the squash.
bool rulesOutNoSelection(const std::string& condition)
{
	std::string squashed;
	for (const char c : condition) {
		if (std::isspace(static_cast<unsigned char>(c)) == 0) {
			squashed += c;
		}
	}
	return squashed.find(kNoSelectionCheck) != std::string::npos;
}

// One subscript of the graph made with the selection: the file it is in, where
// in it, what it is applied to, the expression it was applied to, and the
// condition of every block enclosing it, outermost first.
struct IndexSite
{
	std::string file;
	int line = 0;
	std::string::size_type column = 0;
	std::string array;
	std::string index;
	std::vector<std::string> conditions;
};

// Every subscript in one file whose index expression names the selection,
// carrying the conditions of the blocks each one sits inside.
//
// A site is recorded before the braces on its own line are applied, so a block
// that opens further along the same line cannot be credited to it, and the
// walk runs left to right so a site that precedes a closing brace on the same
// line is not credited with the block that brace ends.
std::vector<IndexSite> findSelectionIndexes(const std::filesystem::path& path)
{
	std::vector<IndexSite> sites;

	const std::vector<std::string> lines = readLines(path);
	const std::string file = displayPath(path);

	std::vector<std::string> code;
	bool inBlockComment = false;
	for (const std::string& line : lines) {
		code.push_back(codeOnly(line, inBlockComment));
	}

	// The condition each currently open brace introduced, outermost first.
	std::vector<std::string> conditions;

	for (std::string::size_type l = 0; l < code.size(); ++l) {
		std::vector<IndexSite> onThisLine;

		for (const char* const array : kGraphArrays) {
			const std::string name = array;
			for (const std::string::size_type at : wholeWordPositions(code[l], name)) {
				const std::string::size_type open = at + name.size();
				if (open >= code[l].size() || code[l][open] != '[') {
					continue;
				}
				const std::string index = subscriptExpression(code[l], open);
				if (wholeWordPositions(index, kSelection).empty()) {
					continue;
				}

				IndexSite site;
				site.file = file;
				site.line = static_cast<int>(l) + 1;
				site.column = at;
				site.array = name;
				site.index = index;
				onThisLine.push_back(site);
			}
		}

		std::size_t next = 0;
		for (std::string::size_type i = 0; i <= code[l].size(); ++i) {
			if (i < code[l].size() && code[l][i] != '{' && code[l][i] != '}') {
				continue;
			}

			while (next < onThisLine.size() && onThisLine[next].column < i) {
				onThisLine[next].conditions = conditions;
				sites.push_back(onThisLine[next]);
				++next;
			}

			if (i == code[l].size()) {
				break;
			}
			if (code[l][i] == '{') {
				conditions.push_back(conditionBeforeBrace(code, l, i));
			}
			else if (!conditions.empty()) {
				conditions.pop_back();
			}
		}
	}

	return sites;
}

// The same sweep over every source file in both trees, in file then line order.
std::vector<IndexSite> findAllSelectionIndexes()
{
	std::vector<IndexSite> sites;

	for (const std::filesystem::path& path : readAllAppSources()) {
		const std::vector<IndexSite> found = findSelectionIndexes(path);
		sites.insert(sites.end(), found.begin(), found.end());
	}

	return sites;
}

// How many of `sites` are in `file`.
std::size_t countIn(const std::vector<IndexSite>& sites, const std::string& file)
{
	std::size_t count = 0;
	for (const IndexSite& site : sites) {
		if (site.file == file) {
			++count;
		}
	}

	return count;
}

std::string describe(const std::vector<IndexSite>& sites)
{
	std::string joined;
	for (const IndexSite& site : sites) {
		if (!joined.empty()) {
			joined += "; ";
		}
		joined += site.file + ":" + std::to_string(site.line) + ": " + site.array +
		          "[" + site.index + "]";
	}
	return joined;
}

// The conditions a site sits inside, for a failure message: the innermost one
// first, because that is the one that was supposed to stop it. A block that is
// not opened by a condition at all - the function body, normally - reads as
// `nothing`, so an unguarded site says so rather than looking like it had one.
std::string describe(const std::vector<std::string>& conditions)
{
	std::string joined;
	for (std::vector<std::string>::const_reverse_iterator condition = conditions.rbegin();
	     condition != conditions.rend(); ++condition) {
		if (!joined.empty()) {
			joined += "; ";
		}
		joined += condition->empty() ? "nothing" : *condition;
	}
	return joined;
}

} // namespace

TEST_CASE("no file indexes the path point graph with an unselected point", "[pathpoint][bounds]")
{
	// Reached with more than one pathfind waypoint and none of them selected,
	// which is the state the editor is in by default after one press of `.`.
	// Nothing upstream repairs it: the sentinel is a resting value, and
	// GameTick.cpp:1676 and :1706 both test for it before they subscript.
	const std::vector<IndexSite> sites = findAllSelectionIndexes();

	SECTION("the scan found the sites it is checking")
	{
		// Non-vacuity. The three in GameDraw.cpp are the x, the y and the z of
		// one glVertex3f, and this is what stops the assertion below from
		// holding because the scan matched nothing - whether because a site was
		// renamed, moved to another file, or dropped, or because the marker
		// point was deleted outright as a way of "fixing" the read.
		INFO("subscripts found with the selection: " << describe(sites));
		REQUIRE(sites.size() == static_cast<std::size_t>(kExpectedSites));
	}

	SECTION("the marker point is still among them")
	{
		REQUIRE(countIn(sites, kMarkerFile) == static_cast<std::size_t>(kExpectedMarkerSites));
	}

	SECTION("every one of them is inside a block guarded against the sentinel")
	{
		for (const IndexSite& site : sites) {
			bool guarded = false;
			for (const std::string& condition : site.conditions) {
				guarded = guarded || rulesOutNoSelection(condition);
			}

			INFO(site.file << ":" << site.line << ": " << site.array << "[" << site.index << "]"
			     << " is inside, innermost first: " << describe(site.conditions));
			REQUIRE(guarded);
		}
	}
}
