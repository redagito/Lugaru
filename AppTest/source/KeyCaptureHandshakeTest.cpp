// Behavioural and architecture test for the key-capture handshake.
//
// Lugaru has exactly one thread. It exists so that clicking a keybind row in the
// controls menu parks the main loop while SDL_WaitEvent blocks for the next key
// or mouse button, and the way the main loop knows to stay parked is two plain
// members of GameState - waiting and keyselect - that the thread writes and the
// main thread polls. Neither was atomic, so the handshake was undefined
// behaviour by the standard, and only behaved because the join happened to
// create the ordering edge after the fact.
//
// They cannot simply be made atomic where they are: std::atomic is not
// copyable, and GameState is asserted to be trivially copyable and trivially
// destructible (GameStateTest.cpp), so that it can be built and destroyed in a
// unit test with no GL context. The owner is KeyCapture in App/include, a header
// of its own, constructed once in main() beside GameState and passed by reference
// down the same chain GameState already travels - no accessor, no global, no
// file-local static.
//
// The second half of the handshake was worse: the thread used to end by calling
// Menu::Load, which clears and rebuilds the file-static Menu::items vector that
// Menu::handleFadeEffect walks on the main thread, and which reaches GameAssets
// textures on its way. The thread now only records the key it captured and asks
// for the menu to be reloaded.
//
// The third member of the handshake, reloadRequested, is what carries that ask
// across the thread boundary, so the tests below pin all three properties that
// matter: the members are atomic, a request is handed over exactly once, and the
// thread never reaches the menu item list itself. The first two are behavioural
// and run here; the third cannot be executed by a unit test - the thread needs a
// window and a GL context to exist - so it is asserted by reading the source as
// text, the way the other ownership tests in this project do.
//
// The test project is told where the sources live (LUGARU_APP_INCLUDE_DIR,
// LUGARU_APP_SOURCE_DIR, LUGARU_LUGARU_SOURCE_DIR) so the scans do not have to
// guess from the working directory.

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "KeyCapture.hpp"

namespace
{

const char* const kGameStateHeader = LUGARU_APP_INCLUDE_DIR "/GameState.hpp";
const char* const kMenuSource = LUGARU_APP_SOURCE_DIR "/Menu/Menu.cpp";
const char* const kMainSource = LUGARU_LUGARU_SOURCE_DIR "/main.cpp";

// The thread body, anchored on the signature rather than a line number so that
// formatting cannot move the check off the thing it is checking.
const char* const kThreadSignature = "int setKeySelected_thread(void* data)";

// The main-thread half of the handshake lives here.
const char* const kTickSignature = "void Menu::Tick(";

// The guard that makes the join unconditional, pinned by name because it is the
// only thing standing between an exception on the main thread and a thread still
// holding references to objects that are being destroyed underneath it.
const char* const kJoinGuard = "JoinKeySelectThreadOnExit";

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
// inside a longer identifier: waiting must not be found inside
// waitingForNothingElse, and keyselect inside keyselectionPrompt.
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

std::vector<std::string> readSplitLines(const char* path)
{
	std::vector<std::string> lines;
	std::string line;

	std::istringstream input(readText(path));
	while (std::getline(input, line)) {
		lines.push_back(line);
	}

	return lines;
}

// Blanks out everything on a line that is not code - // comments, /* */
// comments, and string and character literals - padding with spaces so that what
// is left keeps its original columns. Without this a brace inside a comment
// would move the depth the function body below is matched against, and a
// `Menu::Load` written in prose would read as a call.
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

// A whole file with the comments and literals blanked out.
std::string readCode(const char* path)
{
	std::string code;
	bool inBlockComment = false;

	for (const std::string& line : readSplitLines(path)) {
		code += codeOnly(line, inBlockComment);
		code += '\n';
	}

	return code;
}

// The body of the function whose definition starts at `signature`, matched by
// brace depth so that a nested block does not end it early. Empty when the
// signature is not in the file at all, which every caller turns into a failure -
// a scan that matched nothing must never read as a scan that passed.
std::string functionBody(const std::string& code, const std::string& signature)
{
	const std::string::size_type start = code.find(signature);
	if (start == std::string::npos) {
		return std::string();
	}

	const std::string::size_type open = code.find('{', start + signature.size());
	if (open == std::string::npos) {
		return std::string();
	}

	int depth = 0;
	for (std::string::size_type i = open; i < code.size(); ++i) {
		if (code[i] == '{') {
			++depth;
		}
		else if (code[i] == '}') {
			--depth;
			if (depth == 0) {
				return code.substr(open, i - open + 1);
			}
		}
	}

	return std::string();
}

// Where `declaration` is written in `code`, or npos.
std::string::size_type positionOf(const std::string& code, const std::string& declaration)
{
	return code.find(declaration);
}

} // namespace

TEST_CASE("a fresh KeyCapture has no capture in flight", "[keycapture]")
{
	// Every one of these is what the main loop assumes before the player has
	// clicked anything: it pumps events, it does not skip a row in the controls
	// menu, and there is no menu reload owed to it.
	KeyCapture capture;

	SECTION("the capture thread is not running")
	{
		REQUIRE(capture.waiting == false);
	}

	SECTION("no keybind row is armed")
	{
		// 0 is not -1: the sentinel that means "this row is not waiting for a
		// key" is -1, so 0 leaves the Forwards row nominally selected without any
		// thread behind it. Pinned because that is the value GameState carried
		// before the move, and a default of -1 here would change what the
		// controls menu draws on the first frame it is reached.
		REQUIRE(capture.keyselect == 0);
	}

	SECTION("no menu reload is pending")
	{
		// Consuming an absent request has to be harmless, because Menu::Tick asks
		// once every frame it runs and there is only a request to answer once per
		// completed rebind.
		REQUIRE(capture.takeReloadRequest() == false);
	}
}

TEST_CASE("the key capture handshake is held in atomics", "[keycapture]")
{
	// These two members are written by the capture thread and polled by the main
	// loop. As a plain bool and int that is a data race by the standard, whatever
	// the join later does for the writes that happen before it.
	//
	// std::is_atomic_v would be the obvious way to ask, but the <atomic> this
	// project builds against does not declare the trait at all, so each member is
	// named exactly instead. That is the stronger statement anyway: waiting has to
	// be a std::atomic<bool> and not merely some atomic, and keyselect an
	// std::atomic<int>, because their values are compared against false and
	// against -1 throughout Menu.cpp.
	//
	// The second assertion in each section is the property the trait would have
	// been standing in for, and the one the rest of the design rests on: an atomic
	// cannot be copied. GameState has to stay trivially copyable, so this is
	// precisely why the handshake had to leave it. These are REQUIREs rather than
	// static_asserts so that a regression is a failing test with a name and a
	// message, not a compile error pointing at this file.
	SECTION("waiting")
	{
		REQUIRE(std::is_same<decltype(KeyCapture::waiting), std::atomic<bool>>::value);
		REQUIRE_FALSE(std::is_copy_constructible_v<decltype(KeyCapture::waiting)>);
	}

	SECTION("keyselect")
	{
		REQUIRE(std::is_same<decltype(KeyCapture::keyselect), std::atomic<int>>::value);
		REQUIRE_FALSE(std::is_copy_constructible_v<decltype(KeyCapture::keyselect)>);
	}

	SECTION("reloadRequested")
	{
		REQUIRE(std::is_same<decltype(KeyCapture::reloadRequested), std::atomic<bool>>::value);
		REQUIRE_FALSE(std::is_copy_constructible_v<decltype(KeyCapture::reloadRequested)>);
	}

	SECTION("so the owner cannot be copied either")
	{
		// The consequence of the three above, and the reason the owner had to be
		// a separate object rather than two members of GameState. Default
		// construction still has to work, because main() default constructs one.
		REQUIRE_FALSE(std::is_copy_constructible_v<KeyCapture>);
		REQUIRE_FALSE(std::is_copy_assignable_v<KeyCapture>);
		REQUIRE(std::is_default_constructible_v<KeyCapture>);
	}
}

TEST_CASE("a reload request reaches the main thread exactly once", "[keycapture]")
{
	// The request is the whole of what the capture thread now asks the main
	// thread to do. If takeReloadRequest() could report the same request twice
	// the menu would be rebuilt twice from one rebind; if it swallowed the
	// request the controls menu would never show the key that was just captured.
	// Both failure modes are silent, so the transition itself is what is tested
	// rather than a count.
	KeyCapture capture;

	SECTION("nothing to take means nothing taken")
	{
		REQUIRE(capture.takeReloadRequest() == false);
	}

	SECTION("one request is taken once")
	{
		capture.reloadRequested = true;

		REQUIRE(capture.takeReloadRequest() == true);
		REQUIRE(capture.takeReloadRequest() == false);
		REQUIRE(capture.takeReloadRequest() == false);
	}

	SECTION("a second rebind is taken separately")
	{
		// The consume is a reset, not a one-shot latch: two captures in one
		// session have to be able to each ask for their own reload.
		capture.reloadRequested = true;
		REQUIRE(capture.takeReloadRequest() == true);

		capture.reloadRequested = true;
		REQUIRE(capture.takeReloadRequest() == true);
		REQUIRE(capture.takeReloadRequest() == false);
	}

	SECTION("the request survives the flag being cleared while one is pending")
	{
		// takeReloadRequest() clears the flag as it consumes it, and the thread
		// sets it before clearing waiting. A consume that raced the set must not
		// lose it, so the flag is cleared underneath a pending request and the
		// request has to still be there afterwards.
		capture.reloadRequested = true;
		capture.reloadRequested = false;
		REQUIRE(capture.takeReloadRequest() == false);

		capture.reloadRequested = true;
		REQUIRE(capture.takeReloadRequest() == true);
	}
}

TEST_CASE("GameState no longer carries the capture handshake", "[keycapture][architecture]")
{
	// Not a style rule: GameState has to stay trivially copyable so it can be
	// constructed in a unit test with no GL context, and a std::atomic member
	// would take that away. So the handshake cannot live here at all, which is
	// what this pins - a fix that made them atomic in place would not build.
	const std::string code = readCode(kGameStateHeader);

	SECTION("the header was read and still has the members it should")
	{
		// Non-vacuity. If the scan had found nothing, the two assertions below
		// would hold over an empty string, and an emptied or relocated header
		// would look exactly like a fixed one.
		INFO("GameState.hpp is " << code.size() << " bytes of code");
		REQUIRE(code.size() > 1000);
		REQUIRE_FALSE(code.find("struct GameState") == std::string::npos);
		REQUIRE_FALSE(wholeWordPositions(code, "editoractive").empty());
	}

	SECTION("keyselect is gone")
	{
		INFO("occurrences of keyselect: " << wholeWordPositions(code, "keyselect").size());
		REQUIRE(wholeWordPositions(code, "keyselect").empty());
	}

	SECTION("waiting is gone")
	{
		INFO("occurrences of waiting: " << wholeWordPositions(code, "waiting").size());
		REQUIRE(wholeWordPositions(code, "waiting").empty());
	}

	SECTION("nothing atomic was put in its place")
	{
		// The move had to take the handshake out of GameState, not re-declare it
		// there with atomic storage.
		INFO("GameState.hpp still names atomic");
		REQUIRE(wholeWordPositions(code, "atomic").empty());
	}
}

TEST_CASE("the capture thread does not reach the menu item list", "[keycapture][architecture]")
{
	// The thread used to end with Menu::Load, which clears and rebuilds the
	// file-static Menu::items that Menu::handleFadeEffect walks on the main
	// thread, and which reads assets.Mainmenuitems and assets.Mapcircletexture
	// while doing it. That is two shared structures written by one thread and
	// read by the other with nothing ordering them.
	//
	// The thread now records the key it captured and asks for the reload; the
	// main thread does it. What is pinned here is that nothing named Load is left
	// in the thread body, which is the only route to those structures it had.
	const std::string code = readCode(kMenuSource);

	SECTION("the file was read and all three functions are in it")
	{
		// Non-vacuity, and it has to be checked before the body assertions: a
		// missing Menu::Load definition or a missing thread body would make "the
		// thread does not call Load" true for the wrong reason.
		INFO("Menu.cpp is " << code.size() << " bytes of code");
		REQUIRE(code.size() > 1000);
		REQUIRE_FALSE(functionBody(code, "void Menu::Load(").empty());
		REQUIRE_FALSE(functionBody(code, kTickSignature).empty());
		REQUIRE_FALSE(functionBody(code, kThreadSignature).empty());
	}

	SECTION("the thread body was found and still does the capture")
	{
		// Non-vacuity on the body itself: an empty body would satisfy "no Load
		// call" on its own, so the body has to be shown to be the real one.
		const std::string body = functionBody(code, kThreadSignature);

		INFO("thread body is " << body.size() << " bytes");
		REQUIRE(body.size() > 100);
		REQUIRE(body.find("SDL_WaitEvent") != std::string::npos);
		REQUIRE(body.find("keyselect") != std::string::npos);
	}

	SECTION("it calls no overload of Menu::Load")
	{
		const std::string body = functionBody(code, kThreadSignature);

		INFO("occurrences of Load in the thread body: " << wholeWordPositions(body, "Load").size());
		REQUIRE(wholeWordPositions(body, "Load").empty());
	}

	SECTION("it no longer reaches Menu::items either")
	{
		// The same escape hatch spelled the other way: a bare call into the item
		// list from the thread would be the same race without the word Load in it.
		const std::string body = functionBody(code, kThreadSignature);

		REQUIRE(wholeWordPositions(body, "items").empty());
		REQUIRE(wholeWordPositions(body, "setText").empty());
		REQUIRE(wholeWordPositions(body, "clearMenu").empty());
	}
}

TEST_CASE("the capture thread is still joined on every exit path", "[keycapture][architecture]")
{
	// A regression guard, not a description of what is being fixed here. The
	// thread holds a reference to the caller's GameState and to the KeyCapture
	// beside it, both stack objects in main(), so it must be joined before either
	// is destroyed. That was a real bug - the enclosing catch block was reachable
	// with the thread still alive - and it was fixed with an RAII guard declared
	// after both objects. Nothing in this change is allowed to make it easy to
	// lose that again, and nothing here would notice if it were simply deleted.
	const std::string code = readCode(kMainSource);

	SECTION("the guard still exists and still joins")
	{
		REQUIRE(code.find("struct " + std::string(kJoinGuard)) != std::string::npos);

		// The destructor is the guard: a bare declaration with an empty body would
		// satisfy the line above and re-open the hole.
		const std::string guard = functionBody(code, "struct " + std::string(kJoinGuard));
		REQUIRE_FALSE(guard.empty());
		REQUIRE(guard.find("Menu::joinKeySelectThread") != std::string::npos);
	}

	SECTION("main() still owns the two objects the thread references")
	{
		// Non-vacuity for the ordering assertions below.
		REQUIRE(positionOf(code, "GameState gamestate;") != std::string::npos);
		REQUIRE(positionOf(code, "GameAssets assets;") != std::string::npos);
		REQUIRE(positionOf(code, "KeyCapture keycapture;") != std::string::npos);
	}

	SECTION("the guard is declared after both of them")
	{
		// Destruction runs in reverse, so a guard declared after the objects it
		// protects is destroyed before them. Declared before, it would join a
		// thread whose references are already dangling - which is exactly the bug
		// 2c recorded.
		const std::string::size_type gamestate = positionOf(code, "GameState gamestate;");
		const std::string::size_type assets = positionOf(code, "GameAssets assets;");
		const std::string::size_type keycapture = positionOf(code, "KeyCapture keycapture;");
		const std::string::size_type guard = positionOf(code, kJoinGuard + std::string(" joinKeySelectThread;"));

		INFO("gamestate at " << gamestate << ", assets at " << assets
		     << ", keycapture at " << keycapture << ", guard at " << guard);
		REQUIRE(gamestate < assets);
		REQUIRE(assets < keycapture);
		REQUIRE(keycapture < guard);
	}

	SECTION("the explicit join before deleteGame is still there")
	{
		// The guard covers the paths that never reach the end of the loop; this
		// one is still needed because deleteGame drops GL objects the thread could
		// otherwise be reading. Removing it would make the join happen after the
		// context is gone.
		const std::vector<std::string::size_type> joins = wholeWordPositions(code, "joinKeySelectThread");
		INFO("occurrences of joinKeySelectThread: " << joins.size());
		REQUIRE(joins.size() >= 2);
	}
}