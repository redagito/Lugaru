// Unit tests for the Foundation layer.
//
// References FoundationLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "Utils/binio.h"
#include "Utils/private.h"

namespace
{

// Round-trips values through a std::vector-backed buffer.
template <typename Writer, typename Reader>
void roundTrip(Writer pack, Reader unpack)
{
	std::vector<unsigned char> buffer(256, 0);
	pack(buffer.data());
	unpack(buffer.data());
}

} // namespace

TEST_CASE("BinIOFormatByteCount", "[binio]")
{
	SECTION("returns 0 for an empty format")
	{
		REQUIRE(BinIOFormatByteCount("") == 0u);
	}

	SECTION("counts a single value")
	{
		REQUIRE(BinIOFormatByteCount("b") == 1u);
		REQUIRE(BinIOFormatByteCount("s") == 2u);
		REQUIRE(BinIOFormatByteCount("i") == 4u);
		REQUIRE(BinIOFormatByteCount("l") == 8u);
		REQUIRE(BinIOFormatByteCount("f") == 4u);
		REQUIRE(BinIOFormatByteCount("d") == 8u);
	}

	SECTION("ignored bytes still consume space in the stream")
	{
		REQUIRE(BinIOFormatByteCount("x") == 1u);
	}

	SECTION("sums across mixed fields")
	{
		REQUIRE(BinIOFormatByteCount("bi") == 5u);
		REQUIRE(BinIOFormatByteCount("bid") == 13u);
		REQUIRE(BinIOFormatByteCount("ii") == 8u);
	}

	SECTION("handles a run of fields")
	{
		REQUIRE(BinIOFormatByteCount("bbbbbbbb") == 8u);
	}
}

TEST_CASE("pack and unpack round-trip", "[binio]")
{
	SECTION("integers survive a pack/unpack cycle")
	{
		unsigned char byte = 0;
		short sint = 0;
		int integer = 0;
		long long wide = 0;

		roundTrip(
			[](void* buffer) { spackf(buffer, "bsil", (uint8_t)0xAB, (int16_t)-1234, (int32_t)-567890, (int64_t)-1234567890123LL); },
			[&](const void* buffer) { sunpackf(buffer, "bsil", &byte, &sint, &integer, &wide); });

		REQUIRE(byte == 0xAB);
		REQUIRE(sint == -1234);
		REQUIRE(integer == -567890);
		REQUIRE(wide == -1234567890123LL);
	}

	SECTION("floats survive a pack/unpack cycle")
	{
		float single = 0.0f;
		double dbl = 0.0;

		roundTrip(
			[](void* buffer) { spackf(buffer, "fd", (float)1.5f, (double)-2.25); },
			[&](const void* buffer) { sunpackf(buffer, "fd", &single, &dbl); });

		REQUIRE(single == 1.5f);
		REQUIRE(dbl == -2.25);
	}

	SECTION("byte order matches the host")
	{
		const int32_t value = 0x01020304;
		unsigned char buffer[4] = { 0, 0, 0, 0 };
		spackf(buffer, "i", value);

		int32_t readBack = 0;
		sunpackf(buffer, "i", &readBack);
		REQUIRE(readBack == value);
	}
}

TEST_CASE("funpackf rejects truncated input", "[binio]")
{
	// Regression test: vfunpackf used to malloc and fread without checking
	// either result, so a short file silently parsed uninitialised heap memory
	// into caller variables.
	const std::string path = "lugaru-binio-truncated.bin";

	SECTION("a short file must not silently produce garbage")
	{
		FILE* file = std::fopen(path.c_str(), "wb");
		REQUIRE(file != nullptr);
		// "i" needs 4 bytes; write only 2.
		const unsigned char partial[2] = { 0x01, 0x02 };
		REQUIRE(std::fwrite(partial, 1, sizeof(partial), file) == sizeof(partial));
		std::fclose(file);

		file = std::fopen(path.c_str(), "rb");
		REQUIRE(file != nullptr);
		int32_t value = 0;
		REQUIRE_THROWS(funpackf(file, "i", &value));
		std::fclose(file);
		std::remove(path.c_str());
	}

	SECTION("a complete file reads back correctly")
	{
		FILE* file = std::fopen(path.c_str(), "wb");
		REQUIRE(file != nullptr);
		const int32_t expected = 0x0BADF00D;
		fpackf(file, "i", expected);
		std::fclose(file);

		file = std::fopen(path.c_str(), "rb");
		REQUIRE(file != nullptr);
		int32_t value = 0;
		REQUIRE_NOTHROW(funpackf(file, "i", &value));
		std::fclose(file);
		std::remove(path.c_str());

		REQUIRE(value == expected);
	}

	SECTION("reading past end of file is reported")
	{
		FILE* file = std::fopen(path.c_str(), "wb");
		REQUIRE(file != nullptr);
		std::fclose(file);

		file = std::fopen(path.c_str(), "rb");
		REQUIRE(file != nullptr);
		int32_t value = 0;
		REQUIRE_THROWS(funpackf(file, "i", &value));
		std::fclose(file);
		std::remove(path.c_str());
	}
}