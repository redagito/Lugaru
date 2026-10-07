// Unit tests for locating a map in either of the two Maps folders.
//
// References FoundationLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <string>

#include "Platform/Platform.hpp"
#include "Utils/Folders.hpp"

#ifdef LUGARU_PLATFORM_WINDOWS
	#include <direct.h>
#else
	#include <unistd.h>
#endif

namespace
{

// Removes a directory, ignoring the failure that happens when it is not empty
// or is already gone.
void removeDir(const std::string& path)
{
#ifdef LUGARU_PLATFORM_WINDOWS
	_rmdir(path.c_str());
#else
	rmdir(path.c_str());
#endif
}

bool writeFile(const std::string& path, const char* contents)
{
	FILE* file = std::fopen(path.c_str(), "wb");
	if (file == nullptr) {
		return false;
	}
	const std::size_t length = std::char_traits<char>::length(contents);
	const bool ok = std::fwrite(contents, 1, length, file) == length;
	std::fclose(file);
	return ok;
}

// A pair of Maps folders under a temporary root, so the lookup can be exercised
// without touching the real user data or resource folders.
struct TempMaps
{
	std::string root = "lugaru-map-lookup-test";
	std::string userMaps = root + "/user/Maps";
	std::string resourceMaps = root + "/resource/Maps";

	TempMaps()
	{
		Folders::makeDirectory(root);
		Folders::makeDirectory(root + "/user");
		Folders::makeDirectory(root + "/resource");
		Folders::makeDirectory(userMaps);
		Folders::makeDirectory(resourceMaps);
	}

	~TempMaps()
	{
		removeDir(userMaps);
		removeDir(resourceMaps);
		removeDir(root + "/user");
		removeDir(root + "/resource");
		removeDir(root);
	}

	std::string write(const std::string& mapsDir, const std::string& fileName) const
	{
		const std::string path = mapsDir + '/' + fileName;
		writeFile(path, "{}");
		return path;
	}

	std::string resolve(const std::string& name, const std::string& extension) const
	{
		return Folders::findMapPath(userMaps, resourceMaps, name, extension);
	}
};

} // namespace

TEST_CASE("looking a map up in a pair of Maps folders", "[folders]")
{
	const TempMaps maps;

	SECTION("a map is found in whichever folder holds it")
	{
		const std::string userCopy = maps.write(maps.userMaps, "temp.json");
		const std::string resourceCopy = maps.write(maps.resourceMaps, "temp");
		REQUIRE(maps.resolve("temp", ".json") == userCopy);
		REQUIRE(maps.resolve("temp", "") == resourceCopy);
	}

	SECTION("a user map wins over a shipped map of the same name")
	{
		maps.write(maps.resourceMaps, "shadowed.json");
		const std::string userCopy = maps.write(maps.userMaps, "shadowed.json");
		REQUIRE(maps.resolve("shadowed", ".json") == userCopy);
	}

	SECTION("a shipped map is found when the user has no copy")
	{
		const std::string resourceCopy = maps.write(maps.resourceMaps, "shipped.json");
		REQUIRE(maps.resolve("shipped", ".json") == resourceCopy);
	}

	SECTION("a map in neither folder is reported as absent")
	{
		REQUIRE(maps.resolve("missing", ".json").empty());
	}

	SECTION("the extension is part of the file name that is looked for")
	{
		const std::string jsonCopy = maps.write(maps.userMaps, "both.json");
		REQUIRE(maps.resolve("both", ".json") == jsonCopy);
		REQUIRE(maps.resolve("both", "").empty());
	}
}

TEST_CASE("a map written where the editor saves one is found there", "[folders]")
{
	// Regression test: the editor's save commands write to the Maps folder in
	// the user data directory, which the loader used to never look in.
	const std::string mapsDir = Folders::getUserMapsPath();
	REQUIRE(Folders::makeDirectory(mapsDir));

	const std::string name = "lugaru-round-trip-test";
	const std::string savedPath = mapsDir + '/' + name + ".json";
	REQUIRE_FALSE(Folders::file_exists(savedPath));
	REQUIRE(writeFile(savedPath, "{\"version\":13}"));

	REQUIRE(Folders::findMapPath(name, ".json") == savedPath);

	std::remove(savedPath.c_str());
	removeDir(mapsDir);
	REQUIRE_FALSE(Folders::file_exists(savedPath));
}
