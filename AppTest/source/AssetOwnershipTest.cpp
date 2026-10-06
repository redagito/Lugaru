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
// The same reasoning covers the eleven shared textures that used to sit in
// GameGlobals.h beside them. A Texture is a shared_ptr to a GL object, so
// declaring one extern put the whole handle in reach of every file with no
// owner, and GameState cannot take it over for the same reason as the three
// above. They join GameAssets, by value, which is why the header must show no
// pointer to any of them either.
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
#include <type_traits>
#include <vector>

#include "GameAssets.hpp"

namespace
{

const char* const kIncludeDir = LUGARU_APP_INCLUDE_DIR;
const char* const kGameAssetsHeader = LUGARU_APP_INCLUDE_DIR "/GameAssets.hpp";
const char* const kGameGlobalsSource = LUGARU_APP_SOURCE_DIR "/GameGlobals.cpp";

// The .cpp trees, where a shared owner is just as easy to reintroduce as in a
// header and much easier to hide: a file-scope static has no declaration
// anywhere else to grep for, and the accessor can be one line in a file nobody
// reads. App/source and Lugaru/source are both listed because the reverted
// Light& light = SharedLight() anti-pattern lived in App/source, and main.cpp -
// the other place that owns a GameAssets - is not in App.
const char* const kSourceDirs[] = {
	LUGARU_APP_SOURCE_DIR,
	LUGARU_LUGARU_SOURCE_DIR,
};

// The eleven names that used to be declared extern in GameGlobals.h and defined
// in GameGlobals.cpp. Each is paired with the declaration it has as a GameAssets
// member, because "is it a member" is a different, weaker question than "is it
// a member by value".
struct TextureGlobal
{
	const char* name;
	const char* declaration;
};

const TextureGlobal kTextureGlobals[] = {
	{ "terraintexture", "Texture terraintexture;" },
	{ "terraintexture2", "Texture terraintexture2;" },
	{ "loadscreentexture", "Texture loadscreentexture;" },
	{ "Mapcircletexture", "Texture Mapcircletexture;" },
	{ "Maparrowtexture", "Texture Maparrowtexture;" },
	{ "Mapboxtexture", "Texture Mapboxtexture;" },
	{ "cursortexture", "Texture cursortexture;" },
	{ "hawktexture", "Texture hawktexture;" },
	{ "Mainmenuitems", "Texture Mainmenuitems[10];" },
	{ "screentexture", "GLuint screentexture" },
	{ "screentexture2", "GLuint screentexture2" },
};

// The four Model globals, paired with the declaration each one has to end up
// with, for the same reason as the table above: "is it a member" is a weaker
// question than "is it a member by value". These four are named in full because
// Model* is legitimate elsewhere - Person::SphereCheck and Decal both take one -
// so the sweep below has to be about these names, not about the type.
struct ModelGlobal
{
	const char* name;
	const char* declaration;
};

const ModelGlobal kModelGlobals[] = {
	{ "hawk", "Model hawk;" },
	{ "eye", "Model eye;" },
	{ "cornea", "Model cornea;" },
	{ "iris", "Model iris;" },
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

// Every file under `root` whose extension is one of `extensions`, so a
// declaration added somewhere this file has never heard of is still caught.
// Returns the paths it managed to read, sorted so a failure names them in a
// stable order.
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

// Every header in App/include.
std::vector<std::filesystem::path> readAllHeaders()
{
	return readAllFiles(kIncludeDir, { ".h", ".hpp" });
}

// Every implementation file in the .cpp trees.
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

// The files among `paths` that hold shared storage for the owner: a static
// instance, or a name that hands one back. Whole-word throughout, so
// staticassert and a comment do not trip it.
std::vector<std::string> sharedOwnerFiles(const std::vector<std::filesystem::path>& paths)
{
	std::vector<std::string> offenders;

	for (const std::filesystem::path& path : paths) {
		const std::string text = readText(path.string().c_str());
		const bool shares = !wholeWordPositions(text, "static GameAssets").empty() ||
		                    !wholeWordPositions(text, "SharedAssets").empty() ||
		                    !wholeWordPositions(text, "sharedAssets").empty() ||
		                    !wholeWordPositions(text, "gameAssets()").empty();
		if (shares) {
			offenders.push_back(path.string());
		}
	}

	return offenders;
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
	// to arrive as a parameter, so no file may declare shared storage for it or
	// hand one back.
	SECTION("no header declares shared storage for it")
	{
		const std::vector<std::string> offenders = sharedOwnerFiles(readAllHeaders());

		INFO("headers exposing a shared owner: " << joinPaths(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("no source file declares shared storage for it")
	{
		// Headers are where the old external storage was declared, so a header scan
		// is where the audit looked. A file-scope static and its one-line accessor
		// need no header at all, though, and that is exactly how the reverted
		// Light& light = SharedLight() was written.
		const std::vector<std::filesystem::path> sources = readAllSources();
		REQUIRE(sources.size() > 20);

		const std::vector<std::string> offenders = sharedOwnerFiles(sources);

		INFO("sources exposing a shared owner: " << joinPaths(offenders));
		REQUIRE(offenders.empty());
	}
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
		REQUIRE(text.find("consoletext") != std::string::npos);
		REQUIRE(text.find("namespace Game") != std::string::npos);
	}
}

TEST_CASE("the shared textures are members of GameAssets, not globals", "[assets][architecture]")
{
	SECTION("no header declares one of them extern")
	{
		// An extern declaration is what let the draw and load paths name these
		// with no owner in sight. The owner is a struct passed by reference now,
		// so nothing under App/include may declare them at all.
		std::vector<std::string> offenders;
		for (const std::filesystem::path& path : readAllHeaders()) {
			const std::string text = readText(path.string().c_str());
			for (const TextureGlobal& global : kTextureGlobals) {
				if (text.find(std::string("extern Texture ") + global.name) != std::string::npos ||
				    text.find(std::string("extern GLuint ") + global.name) != std::string::npos) {
					offenders.push_back(path.filename().string() + ": " + global.name);
				}
			}
		}

		INFO("headers still declaring a texture global: " << joinPaths(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("no header points at one of them")
	{
		// Stricter than the extern sweep on purpose: a Texture* member of some
		// other owner, or a forward declaration kept only so a header can name
		// one, is the same escape hatch spelled differently.
		std::vector<std::string> offenders;
		for (const std::filesystem::path& path : readAllHeaders()) {
			const std::string text = readText(path.string().c_str());
			if (text.find("Texture*") != std::string::npos ||
			    text.find("GLuint*") != std::string::npos) {
				offenders.push_back(path.filename().string());
			}
		}

		INFO("headers still pointing at a texture: " << joinPaths(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("GameAssets holds every one of them by value")
	{
		REQUIRE(std::filesystem::exists(kGameAssetsHeader));

		// Whole-word, not find: "GLuint screentexture" is a prefix of
		// "GLuint screentexture2", so a plain substring search found one member
		// and reported both as present.
		const std::string text = readText(kGameAssetsHeader);
		for (const TextureGlobal& global : kTextureGlobals) {
			CAPTURE(global.name);
			INFO("missing declaration: " << global.declaration);
			REQUIRE_FALSE(wholeWordPositions(text, global.declaration).empty());
		}
	}

	SECTION("GameAssets reaches none of them through a pointer")
	{
		// The assertion that pins "by value". A Texture* or GLuint* member would
		// compile, draw and load exactly like the globals it replaced, and
		// nothing else here would notice the owner had gone missing again.
		const std::string text = readText(kGameAssetsHeader);

		INFO("GameAssets.hpp still reaches a texture through a pointer");
		REQUIRE(text.find("Texture*") == std::string::npos);
		REQUIRE(text.find("GLuint*") == std::string::npos);
	}
}

TEST_CASE("GameGlobals.cpp no longer defines the shared textures", "[assets][architecture]")
{
	// The definitions sat next to the models still pending migration, which made
	// them easy to leave behind. Only the eleven may have gone: anything else in
	// that file is a later tranche, and the empty-file guard above still catches
	// a wholesale rewrite.
	const std::string text = readText(kGameGlobalsSource);

	SECTION("none of the eleven is left")
	{
		for (const TextureGlobal& global : kTextureGlobals) {
			CAPTURE(global.name);
			REQUIRE(wholeWordPositions(text, global.name).empty());
		}
	}

	SECTION("consoletext is the only thing left in it")
	{
		// The four models that used to sit here have moved to GameAssets, so the
		// file is down to a single pending definition. Anchoring on that exact line
		// is what keeps the sweep above from passing over an emptied file, which is
		// the one way to satisfy "none of the eleven is left" without doing the
		// work - exactly as the hawk and iris anchors did before.
		REQUIRE(text.find("std::string consoletext[15] = {};") != std::string::npos);
		REQUIRE(text.find("namespace Game") != std::string::npos);
	}
}

TEST_CASE("the model globals are members of GameAssets, not globals", "[assets][architecture]")
{
	// hawk, eye, cornea and iris were the last four Model instances in
	// GameGlobals.h. A Model owns four malloc'd buffers through raw pointers and
	// has a destructor that frees them, so it is emphatically not something
	// GameState can hold: that would break the trivial copyability and
	// trivial destructibility GameStateTest.cpp:109-110 asserts, and the implicit
	// copy would free the same buffer twice. GameAssets is the owner for them, by
	// value, like the textures above.
	SECTION("no header declares one of them extern")
	{
		// An extern declaration is what let the load and draw paths name these with
		// no owner in sight. The owner is a struct passed by reference now, so
		// nothing under App/include may declare them at all.
		std::vector<std::string> offenders;
		for (const std::filesystem::path& path : readAllHeaders()) {
			const std::string text = readText(path.string().c_str());
			for (const ModelGlobal& global : kModelGlobals) {
				CAPTURE(global.name);
				REQUIRE(text.find(std::string("extern Model ") + global.name) == std::string::npos);
			}
			if (text.find("extern Model ") != std::string::npos) {
				offenders.push_back(path.filename().string());
			}
		}

		INFO("headers still declaring a model global: " << joinPaths(offenders));
		REQUIRE(offenders.empty());
	}

	SECTION("no header points at one of them")
	{
		// Both spellings, because both dodge the extern sweep above while putting
		// the same unowned model back in reach: a Model* member of some other owner
		// would own nothing and hand back a dangling pointer after the real owner
		// went away. Model* itself is left alone - Person::SphereCheck and Decal
		// legitimately take one, so only these four names are swept.
		for (const std::filesystem::path& path : readAllHeaders()) {
			const std::string text = readText(path.string().c_str());
			for (const ModelGlobal& global : kModelGlobals) {
				CAPTURE(global.name);
				REQUIRE(text.find(std::string("Model* ") + global.name) == std::string::npos);
				REQUIRE(text.find(std::string("Model ") + global.name + "*") == std::string::npos);
			}
		}
	}

	SECTION("GameAssets holds every one of them by value")
	{
		REQUIRE(std::filesystem::exists(kGameAssetsHeader));

		const std::string text = readText(kGameAssetsHeader);
		for (const ModelGlobal& global : kModelGlobals) {
			CAPTURE(global.name);
			INFO("missing declaration: " << global.declaration);
			REQUIRE_FALSE(wholeWordPositions(text, global.declaration).empty());
		}
	}

	SECTION("GameAssets reaches none of them through a pointer")
	{
		// The assertion that pins "by value", and unlike the header sweep above it
		// can be unconditional: nothing in this header has any business holding a
		// Model*. A Model* member would load, scale and draw exactly like the
		// globals it replaced and nothing else here would notice.
		const std::string text = readText(kGameAssetsHeader);

		INFO("GameAssets.hpp still reaches a model through a pointer");
		REQUIRE(text.find("Model*") == std::string::npos);
	}
}

TEST_CASE("GameAssets cannot be copied", "[assets][architecture]")
{
	// The four models are held by value and a Model owns four malloc'd buffers
	// through raw pointers, freeing them in its destructor. The copy constructor
	// GameAssets inherits by default would therefore hand two instances the same
	// four pointers, and both destructors would free them: heap corruption, from a
	// line that compiles cleanly.
	//
	// Nothing copies a GameAssets today. The only two instances are locals passed
	// by reference - Lugaru/source/main.cpp and the Person test helper - so the
	// trap is latent rather than live. Deleting the copy operations is what turns
	// the first `GameAssets b = a;` into a compile error instead.
	SECTION("copy construction is not available")
	{
		REQUIRE_FALSE(std::is_copy_constructible_v<GameAssets>);
	}

	SECTION("copy assignment is not available")
	{
		REQUIRE_FALSE(std::is_copy_assignable_v<GameAssets>);
	}

	SECTION("default construction still is")
	{
		// Guards the two assertions above against passing because GameAssets is
		// simply unusable: the fix has to forbid the copy and leave the rest alone,
		// because both live instances are default constructed.
		REQUIRE(std::is_default_constructible_v<GameAssets>);
	}
}
