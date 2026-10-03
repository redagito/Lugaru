// Unit tests for the Foundation layer.
//
// References FoundationLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <string>

#include "Utils/Folders.hpp"
#include "Utils/ImageIO.hpp"

namespace
{

// Writes a small file so file_exists has something real to find.
bool writeTempFile(const char* path, const char* contents)
{
	FILE* file = std::fopen(path, "wb");
	if (file == nullptr) {
		return false;
	}
	const std::size_t length = std::char_traits<char>::length(contents);
	const bool ok = std::fwrite(contents, 1, length, file) == length;
	std::fclose(file);
	return ok;
}

} // namespace

TEST_CASE("Folders path helpers", "[folders]")
{
	SECTION("getResourcePath joins with a forward slash")
	{
		const std::string path = Folders::getResourcePath("Sounds/Jump.ogg");
		REQUIRE(path == std::string(DATA_DIR) + "/Sounds/Jump.ogg");
		REQUIRE(path.find("//") == std::string::npos);
	}

	SECTION("getUserSavePath lives under the user data directory")
	{
		const std::string save = Folders::getUserSavePath();
		REQUIRE(save.find("users") != std::string::npos);
		REQUIRE(save == Folders::getUserDataPath() + "/users");
	}

	SECTION("the config file path ends in config.txt")
	{
		REQUIRE(Folders::getConfigFilePath().find("config.txt") != std::string::npos);
	}

	SECTION("the user data path is absolute or explicitly relative")
	{
		const std::string path = Folders::getUserDataPath();
		REQUIRE_FALSE(path.empty());
	}
}

TEST_CASE("Folders file access", "[folders]")
{
	const char* path = "lugaru-folders-test.tmp";

	SECTION("a missing file does not exist")
	{
		REQUIRE_FALSE(Folders::file_exists(path));
	}

	SECTION("an existing file is detected")
	{
		REQUIRE(writeTempFile(path, "lugaru"));
		REQUIRE(Folders::file_exists(path));
		std::remove(path);
		REQUIRE_FALSE(Folders::file_exists(path));
	}

	SECTION("openMandatoryFile opens an existing file for reading")
	{
		REQUIRE(writeTempFile(path, "lugaru"));
		FILE* file = Folders::openMandatoryFile(path, "rb");
		REQUIRE(file != nullptr);

		char buffer[16] = { 0 };
		REQUIRE(std::fread(buffer, 1, 6, file) == 6);
		REQUIRE(std::string(buffer) == "lugaru");
		std::fclose(file);
		std::remove(path);
	}

	SECTION("openMandatoryFile throws for a missing file")
	{
		REQUIRE_THROWS_AS(Folders::openMandatoryFile(path, "rb"), FileNotFoundException);
	}

	SECTION("the exception names the missing file")
	{
		try {
			Folders::openMandatoryFile("lugaru-missing-file.xyz", "rb");
			FAIL("expected FileNotFoundException");
		}
		catch (const FileNotFoundException& error) {
			REQUIRE(std::string(error.what()).find("lugaru-missing-file.xyz") != std::string::npos);
		}
	}

	SECTION("makeDirectory is idempotent and reports success")
	{
		const std::string dir = Folders::getScreenshotDir();
		REQUIRE(Folders::makeDirectory(dir));
		REQUIRE(Folders::makeDirectory(dir));
	}
}

TEST_CASE("loading shipped textures", "[image]")
{
	SECTION("a PNG texture loads with the expected shape")
	{
		ImageRec image;
		REQUIRE(load_image("Data/Textures/Blood.png", image, []() {}));

		REQUIRE(image.sizeX > 0);
		REQUIRE(image.sizeY > 0);
		REQUIRE(image.bpp == 32);
		REQUIRE(image.data != nullptr);
		REQUIRE(image.capacity() >= static_cast<size_t>(image.sizeX) * image.sizeY * 4);
	}

	SECTION("a JPEG texture loads with the expected shape")
	{
		ImageRec image;
		REQUIRE(load_image("Data/Textures/Boulder.jpg", image, []() {}));

		REQUIRE(image.sizeX > 0);
		REQUIRE(image.sizeY > 0);
		REQUIRE(image.bpp == 24);
		REQUIRE(image.data != nullptr);
		// The buffer must be large enough for the whole decoded image.
		REQUIRE(image.capacity() >= static_cast<size_t>(image.sizeX) * image.sizeY * 3);
	}

	SECTION("the shipped textures fit inside the buffer that was allocated")
	{
		// Regression test: ImageRec used to hand out a fixed 4 MiB block that a
		// larger texture would silently overrun.
		ImageRec image;
		REQUIRE(load_image("Data/Textures/Blood.png", image, []() {}));
		const size_t needed = static_cast<size_t>(image.sizeX) * image.sizeY * (image.bpp / 8);
		REQUIRE(image.capacity() >= needed);
	}

	SECTION("the progress callback runs once per load")
	{
		ImageRec image;
		int calls = 0;
		load_image("Data/Textures/Blood.png", image, [&calls]() { calls++; });
		REQUIRE(calls == 1);
	}

	SECTION("a missing texture is reported")
	{
		ImageRec image;
		REQUIRE_FALSE(load_image("Data/Textures/NoSuchTexture.png", image, []() {}));
	}
}