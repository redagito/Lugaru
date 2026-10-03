// Unit tests for the Foundation layer.
//
// References FoundationLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

#include "Utils/ImageIO.hpp"

TEST_CASE("ImageRec allocation", "[image]")
{
	SECTION("a new image starts with a usable buffer")
	{
		ImageRec image;
		REQUIRE(image.data != nullptr);
		REQUIRE(image.capacity() > 0u);
		REQUIRE(image.sizeX == 0u);
		REQUIRE(image.sizeY == 0u);
		REQUIRE(image.bpp == 0u);
	}

	SECTION("ensureCapacity is satisfied by the existing buffer")
	{
		ImageRec image;
		const size_t initial = image.capacity();

		REQUIRE(image.ensureCapacity(initial));
		REQUIRE(image.capacity() == initial);
		REQUIRE(image.data != nullptr);
	}

	SECTION("ensureCapacity grows the buffer for a large image")
	{
		ImageRec image;

		// A 2048x2048 RGBA image needs 16 MiB, which exceeds the old fixed
		// 4 MiB allocation and used to overflow it.
		constexpr size_t kLargeImageBytes = 2048u * 2048u * 4u;
		REQUIRE(image.ensureCapacity(kLargeImageBytes));
		REQUIRE(image.capacity() >= kLargeImageBytes);
		REQUIRE(image.data != nullptr);
	}

	SECTION("growing preserves the buffer identity when possible")
	{
		ImageRec image;
		uint8_t* original = image.data;
		image.data[0] = 0xAB;

		REQUIRE(image.ensureCapacity(image.capacity() + 4096));

		// realloc may move the block, but if it did not the contents survive.
		if (image.data == original) {
			REQUIRE(image.data[0] == 0xAB);
		}
		REQUIRE(image.capacity() >= 4096);
	}

	SECTION("an ImageRec cannot be copied")
	{
		// Copying would double-free the malloc'd buffer.
		REQUIRE_FALSE(std::is_copy_constructible<ImageRec>::value);
		REQUIRE_FALSE(std::is_copy_assignable<ImageRec>::value);
	}
}

TEST_CASE("load_image rejects unsupported input", "[image]")
{
	SECTION("a missing file returns false")
	{
		ImageRec image;
		REQUIRE_FALSE(load_image("this-image-does-not-exist.png", image, []() {}));
	}

	SECTION("an unknown extension returns false")
	{
		ImageRec image;
		REQUIRE_FALSE(load_image("lugaru-test-image.tga", image, []() {}));
	}

	SECTION("the progress callback is invoked")
	{
		ImageRec image;
		int calls = 0;
		load_image("this-image-does-not-exist.png", image, [&calls]() { calls++; });
		REQUIRE(calls == 1);
	}
}