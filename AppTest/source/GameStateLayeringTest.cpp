// Architecture test for what App/include/GameState.hpp is allowed to include.
//
// GameState is the object the whole engine is handed by reference, and it is
// deliberately made of scalars, plain-data arrays and a few small value types
// so it stays trivially copyable and trivially destructible - which is what
// lets GameStateTest.cpp construct and destroy one with no GL context
// (GameStateTest.cpp:109-110). That property is a property of the *header*, so
// it is only worth anything if the header keeps pulling in as little as
// possible.
//
// The one include that is not about GameState's own members is
// "Graphic/Stereo.hpp", there for the two StereoMode members and nothing else.
// The two directions that would cost something are both live: a struct whose
// members come from the renderer cannot be built without the renderer, and an
// App header reaching down into Graphics is a step backwards in the layer order
// Foundation -> Audio -> Graphics -> Game -> App -> Lugaru.
//
// This test reads the header as text and requires no include of it to name a
// header under Graphic/. It also requires the scan to have found the includes
// that are legitimately there, so that a parser matching nothing cannot pass the
// assertion above it.

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

const char* const kGameStateHeader = LUGARU_APP_INCLUDE_DIR "/GameState.hpp";

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

// Every header GameState.hpp includes, written the way the directive spells it -
// "Graphic/Stereo.hpp", "Math/Vector3.hpp". Only quoted includes are read,
// because those are the ones resolved against the layer include directories; an
// angle-bracket include would be a system header and has no path to check.
std::vector<std::string> includedHeaders(const std::string& text)
{
	std::vector<std::string> headers;

	for (const std::string& raw : [&]() {
		std::vector<std::string> lines;
		std::string line;
		std::istringstream linesIn(text);
		while (std::getline(linesIn, line)) {
			lines.push_back(line);
		}
		return lines;
	}()) {
		if (!raw.starts_with("#include \"")) {
			continue;
		}

		const std::string::size_type firstQuote = raw.find('"', 9);
		const std::string::size_type lastQuote = raw.find('"', firstQuote + 1);
		if (firstQuote == std::string::npos || lastQuote == std::string::npos) {
			continue;
		}

		headers.push_back(raw.substr(firstQuote + 1, lastQuote - firstQuote - 1));
	}

	return headers;
}

template <typename Headers>
std::string join(const Headers& headers)
{
	std::string joined;
	for (const std::string& header : headers) {
		joined += (joined.empty() ? "" : ", ");
		joined += header;
	}
	return joined;
}

} // namespace

TEST_CASE("GameState does not include a Graphics header", "[gamestate][architecture]")
{
	const std::vector<std::string> headers = includedHeaders(readText(kGameStateHeader));

	SECTION("the scan found the includes GameState.hpp does have")
	{
		// Without this, an include directive the scan failed to match - or a
		// header that had lost the includes its members need - would leave the
		// assertion below passing over nothing. Three is what the file has once
		// the stereo enum is forward declared: the light, the frustum and the
		// vector.
		INFO("includes found: " << join(headers));
		REQUIRE(headers.size() >= 3);
	}

	SECTION("no include reaches into the Graphics layer")
	{
		std::vector<std::string> graphics;
		for (const std::string& header : headers) {
			if (header.starts_with("Graphic/")) {
				graphics.push_back(header);
			}
		}

		INFO("Graphics includes: " << join(graphics));
		REQUIRE(graphics.empty());
	}
}