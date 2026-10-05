// Architecture test for who owns the main window's SDL_Window*.
//
// The handle used to be a global named `sdlwindow` declared three times over:
// an extern in App/include/Globals.h, its definition in App/source/Globals.cpp,
// and a function-local `extern` inside the inline swap_gl_buffers in
// App/include/Game.hpp, which re-declared it purely so the header could reach
// it. None of that can compile-check away and none of it shows up in the build,
// so this test reads the sources as text and pins the shape that replaced it:
// the storage is private to App/source/WindowContext.cpp, and no header may
// name it.
//
// The test project is told where App/include and App/source live
// (LUGARU_APP_INCLUDE_DIR, LUGARU_APP_SOURCE_DIR) so the scan does not have to
// guess from its working directory.

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

const char* const kIncludeDir = LUGARU_APP_INCLUDE_DIR;
const char* const kGlobalsHeader = LUGARU_APP_INCLUDE_DIR "/Globals.h";
const char* const kGameHeader = LUGARU_APP_INCLUDE_DIR "/Game.hpp";
const char* const kWindowContextHeader = LUGARU_APP_INCLUDE_DIR "/WindowContext.hpp";
const char* const kWindowContextSource = LUGARU_APP_SOURCE_DIR "/WindowContext.cpp";

const char* const kHandleName = "sdlwindow";

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

bool mentionsHandle(const char* path)
{
	return !wholeWordPositions(readText(path), kHandleName).empty();
}

// Every header in App/include, so a declaration added somewhere this file has
// never heard of is still caught. Returns the paths it managed to read.
std::vector<std::filesystem::path> readAllHeaders()
{
	std::vector<std::filesystem::path> paths;
	std::error_code error;

	for (const std::filesystem::directory_entry& entry :
	     std::filesystem::recursive_directory_iterator(kIncludeDir, error)) {
		if (!entry.is_regular_file(error)) {
			continue;
		}
		const std::string extension = entry.path().extension().string();
		if (extension == ".h" || extension == ".hpp") {
			paths.push_back(entry.path());
		}
	}

	if (error) {
		FAIL("could not walk " << kIncludeDir << ": " << error.message());
	}

	std::sort(paths.begin(), paths.end());
	return paths;
}

template <typename Paths>
std::string joinPaths(const Paths& paths)
{
	std::string joined;
	for (const std::string& path : paths) {
		joined += (joined.empty() ? "" : ", ");
		joined += path;
	}
	return joined;
}

} // namespace

TEST_CASE("no header declares the window handle", "[window][architecture]")
{
	// The global was reachable from anywhere, and three separate declarations
	// said so. Anyone - including an inline function in a header - could bind
	// to it without any of these declarations being checked against each other.
	// The replacement is a function in WindowContext, so the storage must not be
	// spelled out in any header at all.
	const std::vector<std::filesystem::path> headers = readAllHeaders();

	SECTION("no header declares it extern")
	{
		std::vector<std::string> offenders;
		for (const std::filesystem::path& path : headers) {
			const std::string text = readText(path.string().c_str());
			if (text.find("extern SDL_Window* " + std::string(kHandleName)) != std::string::npos) {
				offenders.push_back(path.filename().string());
			}
		}

		INFO("headers still declaring the handle extern: " << joinPaths(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("no header names it at all")
	{
		// Stricter on purpose. Globals.h and Game.hpp are the two places it was
		// spelled out, and a header that mentions the name could still be
		// shadowing the accessor or reintroducing a shortcut around it.
		std::vector<std::string> offenders;
		for (const std::filesystem::path& path : headers) {
			if (mentionsHandle(path.string().c_str())) {
				offenders.push_back(path.filename().string());
			}
		}

		INFO("headers still naming " << kHandleName << ": " << joinPaths(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("the scan reached the headers it claims to")
	{
		// Without this, an include directory that failed to walk would leave both
		// assertions above passing over nothing.
		REQUIRE(headers.size() > 10);
	}
}

TEST_CASE("the window handle is private to WindowContext.cpp", "[window][architecture]")
{
	SECTION("WindowContext.hpp never mentions it")
	{
		// The getter is enough for callers; exposing the storage would put the
		// old global straight back, just spelled differently.
		REQUIRE_FALSE(mentionsHandle(kWindowContextHeader));
	}

	SECTION("the handle still exists, as a private static in the .cpp")
	{
		// Guards against passing by deleting the window: the handle has to be
		// somewhere, and a file-scope static gives it internal linkage, so no
		// other translation unit can declare it again.
		const std::string text = readText(kWindowContextSource);
		const std::vector<std::string::size_type> at = wholeWordPositions(text, kHandleName);

		INFO("occurrences of " << kHandleName << " in WindowContext.cpp: " << at.size());
		REQUIRE_FALSE(at.empty());

		REQUIRE(text.find("static SDL_Window* " + std::string(kHandleName)) != std::string::npos);
	}

	SECTION("the two headers it used to live in are clean")
	{
		// Spelled out separately from the sweep above so a failure names the file
		// the migration was supposed to change.
		REQUIRE_FALSE(mentionsHandle(kGlobalsHeader));
		REQUIRE_FALSE(mentionsHandle(kGameHeader));
	}
}