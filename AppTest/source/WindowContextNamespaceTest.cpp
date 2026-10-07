// Architecture test for the namespace WindowContext declares its functions in.
//
// WindowContext.hpp grew the accessors that replaced the global window handle
// (createWindow, mainWindow, createGLContext, isFocused) at global scope,
// alongside the older toggleFullscreen and sdlEventProc. Those names are generic
// enough to collide with anything, and every other facility in this tree is
// namespaced - Game.hpp wraps its free functions in `namespace Game` - so a
// header that declares them at global scope is the odd one out.
//
// Nothing about that is a compile error or a runtime fault, which is why this
// test reads the header as text: it finds every function declaration in the
// file and requires each one to sit inside `namespace WindowContext`. It also
// requires the scan to have found something, so a parser that silently matched
// nothing cannot pass the assertion above it.

#include <catch2/catch_test_macros.hpp>

#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

const char* const kWindowContextHeader = LUGARU_APP_INCLUDE_DIR "/WindowContext.hpp";

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

std::string trim(const std::string& text)
{
	const std::string::size_type first = text.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) {
		return std::string();
	}
	return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

std::vector<std::string> readLines(const std::string& text)
{
	std::vector<std::string> lines;
	std::string line;
	std::istringstream stream(text);
	while (std::getline(stream, line)) {
		lines.push_back(line);
	}
	return lines;
}

// Blanks out everything on a line that is not code - // comments, /* */
// comments, and string and character literals - padding with spaces so what is
// left keeps its original columns. Without it a brace or a parenthesis inside a
// comment would move the brace count and be mistaken for a declaration.
std::string codeOnly(const std::string& line, bool& inBlockComment)
{
	std::string code;

	for (std::string::size_type i = 0; i < line.size();) {
		if (inBlockComment) {
			if (line.compare(i, 2, "*/") == 0) {
				inBlockComment = false;
				++i;
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
			++i;
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

// The identifier immediately before the first parenthesis on a line, which is
// the declared name for every shape this header uses: "SDL_Window*
// mainWindow();" and "bool createGLContext();" alike. Empty when whatever sits
// there is not a name, which is how a control-flow line with a stray
// parenthesis is skipped.
std::string nameBeforeParenthesis(const std::string& code)
{
	const std::string::size_type paren = code.find('(');
	if (paren == std::string::npos) {
		return std::string();
	}

	std::string::size_type end = paren;
	while (end > 0 && (code[end - 1] == ' ' || code[end - 1] == '\t')) {
		--end;
	}

	std::string::size_type start = end;
	while (start > 0) {
		const char c = code[start - 1];
		if (c == '_' || std::isalnum(static_cast<unsigned char>(c)) != 0) {
			--start;
			continue;
		}
		break;
	}

	return code.substr(start, end - start);
}

struct Declaration
{
	std::string name;
	int line = 0;
	bool inWindowContext = false;
};

// Every function declaration in the header, with the namespace each one sits in.
// A declaration is a line of code that ends in a semicolon and carries a name
// in front of a parenthesis - which is what a prototype is and what an
// `#include` line or a braced namespace is not.
std::vector<Declaration> declaredFunctions(const std::string& text)
{
	const std::vector<std::string> lines = readLines(text);

	std::vector<std::string> code;
	code.reserve(lines.size());
	bool inBlockComment = false;
	for (const std::string& line : lines) {
		code.push_back(codeOnly(line, inBlockComment));
	}

	// Brace depth in front of each line, so a line can be placed inside or
	// outside the namespace.
	std::vector<int> depthBefore(lines.size());
	int depth = 0;
	for (std::string::size_type i = 0; i < code.size(); ++i) {
		depthBefore[i] = depth;
		for (const char c : code[i]) {
			if (c == '{') {
				++depth;
			}
			else if (c == '}') {
				--depth;
			}
		}
	}

	// The depth inside `namespace WindowContext`, or -1 when the header does not
	// open one at all.
	int namespaceDepth = -1;
	for (std::string::size_type i = 0; i < code.size(); ++i) {
		if (code[i].find("namespace WindowContext") == std::string::npos) {
			continue;
		}
		for (std::string::size_type j = i; j < code.size(); ++j) {
			if (code[j].find('{') != std::string::npos) {
				namespaceDepth = depthBefore[j] + 1;
				break;
			}
		}
		break;
	}

	std::vector<Declaration> declarations;
	for (std::string::size_type i = 0; i < code.size(); ++i) {
		const std::string declarationText = trim(code[i]);
		if (declarationText.empty() || declarationText.front() == '#' || declarationText.back() != ';') {
			continue;
		}

		const std::string name = nameBeforeParenthesis(declarationText);
		if (name.empty() || std::isdigit(static_cast<unsigned char>(name.front())) != 0) {
			continue;
		}

		Declaration declaration;
		declaration.name = name;
		declaration.line = static_cast<int>(i) + 1;
		declaration.inWindowContext = namespaceDepth > 0 && depthBefore[i] >= namespaceDepth;
		declarations.push_back(declaration);
	}

	return declarations;
}

template <typename Declarations>
std::string joinNames(const Declarations& declarations)
{
	std::string joined;
	for (const Declaration& declaration : declarations) {
		joined += (joined.empty() ? "" : ", ");
		joined += declaration.name + " (line " + std::to_string(declaration.line) + ")";
	}
	return joined;
}

} // namespace

TEST_CASE("WindowContext declares its functions in a namespace", "[window][architecture]")
{
	// Global scope means every one of these names competes with every other
	// translation unit in the process, including headers this project does not
	// own. The rest of the tree does not do that: Game.hpp wraps its free
	// functions in `namespace Game`, and the same window handle is only reachable
	// through WindowContext's own accessor.
	const std::vector<Declaration> declarations = declaredFunctions(readText(kWindowContextHeader));

	SECTION("the header still declares functions for this scan to have found")
	{
		// Without this, a header that had been emptied of declarations - or a
		// parser that had stopped matching them - would leave the assertion below
		// passing over nothing. Six is what the file declares today:
		// createWindow, mainWindow, createGLContext, isFocused, toggleFullscreen
		// and sdlEventProc.
		INFO("declarations found: " << joinNames(declarations));
		REQUIRE(declarations.size() >= 6);
	}

	SECTION("every declaration is inside namespace WindowContext")
	{
		std::vector<Declaration> globals;
		for (const Declaration& declaration : declarations) {
			if (!declaration.inWindowContext) {
				globals.push_back(declaration);
			}
		}

		INFO("declared at global scope: " << joinNames(globals));
		REQUIRE(globals.empty());
	}
}