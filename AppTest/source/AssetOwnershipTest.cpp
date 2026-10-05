// Architecture test for who owns the skybox and the two text objects.
//
// The three used to be raw new/delete pairs behind extern pointers declared in
// App/include/GameGlobals.h and defined in App/source/GameGlobals.cpp:
//
//     SkyBox* skybox = NULL;  Text* text = NULL;  Text* textmono = NULL;
//
// Nothing tied an allocation to its release, so any early return between
// Game::newGame() and Game::deleteGame() leaked all three, and GameState could
// not hold them either: they carry OpenGL handles, and GameState has to stay
// trivially copyable so it can be unit tested without a GL context.
//
// The replacement is GameAssets in App/include/GameAssets.hpp, which holds all
// three by value and is passed by reference from main() down to the load and
// draw paths. Holding them by value is the part worth pinning: a member declared
// as SkyBox* or Text* would compile and run exactly like the old globals, so
// nothing below would notice that the new/delete pair was back.
//
// This test therefore reads the sources as text rather than exercising the
// objects, which cannot be built without a context. The test project is told
// where App/include and App/source live (LUGARU_APP_INCLUDE_DIR,
// LUGARU_APP_SOURCE_DIR) so the scan does not have to guess from its working
// directory.

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
const char* const kGameAssetsHeader = LUGARU_APP_INCLUDE_DIR "/GameAssets.hpp";
const char* const kGameGlobalsSource = LUGARU_APP_SOURCE_DIR "/GameGlobals.cpp";

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
// inside a longer identifier: skybox must not be found in skyboxtexture.
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

TEST_CASE("no header declares the skybox or text objects as globals", "[assets][architecture]")
{
	// An extern pointer in a header is what made these reachable from anywhere,
	// including the draw path, with no owner in sight. The owner is a struct
	// passed by reference now, so no header may name these three at all.
	const std::vector<std::filesystem::path> headers = readAllHeaders();

	SECTION("no header declares them extern")
	{
		std::vector<std::string> offenders;
		for (const std::filesystem::path& path : headers) {
			const std::string text = readText(path.string().c_str());
			if (text.find("extern SkyBox*") != std::string::npos ||
			    text.find("extern Text*") != std::string::npos) {
				offenders.push_back(path.filename().string());
			}
		}

		INFO("headers still declaring one of them extern: " << joinPaths(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("no header declares a pointer to them")
	{
		// Stricter than the extern sweep on purpose: a pointer member of some
		// other owner, or a forward declaration that only exists so a header can
		// mention them, is the same escape hatch spelled differently. Nothing
		// outside GameAssets.hpp should reach these types by pointer.
		std::vector<std::string> offenders;
		for (const std::filesystem::path& path : headers) {
			const std::string text = readText(path.string().c_str());
			if (text.find("SkyBox*") != std::string::npos ||
			    text.find("Text*") != std::string::npos) {
				offenders.push_back(path.filename().string());
			}
		}

		INFO("headers still pointing at one of them: " << joinPaths(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("the scan reached the headers it claims to")
	{
		// Without this, an include directory that failed to walk would leave both
		// assertions above passing over nothing.
		REQUIRE(headers.size() > 10);
	}
}

TEST_CASE("GameAssets owns the three objects by value", "[assets][architecture]")
{
	SECTION("the owner exists as a header of its own")
	{
		REQUIRE(std::filesystem::exists(kGameAssetsHeader));
	}

	const std::string text = readText(kGameAssetsHeader);

	SECTION("it holds the skybox by value")
	{
		REQUIRE(text.find("SkyBox skybox;") != std::string::npos);
	}

	SECTION("it holds the proportional font by value")
	{
		REQUIRE(text.find("Text text;") != std::string::npos);
	}

	SECTION("it holds the monospaced font by value")
	{
		REQUIRE(text.find("Text textmono;") != std::string::npos);
	}

	SECTION("it holds no pointer to any of them")
	{
		// The assertion that pins "no manual new and delete". A SkyBox* or Text*
		// member would restore exactly the ownership hole this migration closed,
		// and it would still compile and still draw.
		INFO("GameAssets.hpp still reaches through a pointer");
		REQUIRE(text.find("SkyBox*") == std::string::npos);
		REQUIRE(text.find("Text*") == std::string::npos);
	}
}

TEST_CASE("nothing reaches the three objects behind a shared accessor", "[assets][architecture]")
{
	// A static owner behind a SharedAssets()-style getter is the same global as
	// the one just removed, only better hidden: it compiles, it is unreachable by
	// grep for the names, and every frame still shares one instance. The owner has
	// to arrive as a parameter, so no header may declare shared storage for it or
	// a function that hands one back.
	std::vector<std::string> offenders;

	for (const std::filesystem::path& path : readAllHeaders()) {
		const std::string text = readText(path.string().c_str());
		const bool shares = wholeWordPositions(text, "static GameAssets").size() != 0 ||
		                    wholeWordPositions(text, "SharedAssets").size() != 0 ||
		                    wholeWordPositions(text, "sharedAssets").size() != 0 ||
		                    wholeWordPositions(text, "gameAssets()").size() != 0;
		if (shares) {
			offenders.push_back(path.filename().string());
		}
	}

	INFO("headers exposing a shared owner: " << joinPaths(offenders));
	REQUIRE(offenders.empty());
}

TEST_CASE("GameGlobals.cpp no longer defines the three objects", "[assets][architecture]")
{
	// The definitions used to sit next to the textures and models still pending
	// migration, which made them easy to leave behind. Only the ones that are
	// gone may have vanished: anything else in that file is a later tranche.
	const std::string text = readText(kGameGlobalsSource);

	const std::vector<std::string::size_type> skybox = wholeWordPositions(text, "skybox");
	const std::vector<std::string::size_type> mono = wholeWordPositions(text, "textmono");

	INFO("occurrences of skybox: " << skybox.size() << ", of textmono: " << mono.size());
	REQUIRE(skybox.empty());
	REQUIRE(mono.empty());

	SECTION("no declaration of the proportional font either")
	{
		// Whole-word, so consoletext and text[m] in a comment would not trip it.
		const std::vector<std::string::size_type> proportional = wholeWordPositions(text, "text");
		INFO("occurrences of text: " << proportional.size());
		REQUIRE(proportional.empty());
	}

	SECTION("the rest of the file is untouched")
	{
		// Guards against 'remove the globals' being done by emptying the file.
		REQUIRE(text.find("Mainmenuitems") != std::string::npos);
		REQUIRE(text.find("namespace Game") != std::string::npos);
	}
}