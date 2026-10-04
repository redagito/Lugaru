// Unit tests for the Foundation layer.
//
// References FoundationLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "Utils/binio.h"
#include "Utils/private.h"

namespace
{

std::vector<uint8_t> bytesOf(const std::string& path)
{
	FILE* file = std::fopen(path.c_str(), "rb");
	REQUIRE(file != nullptr);
	std::fseek(file, 0, SEEK_END);
	const long size = std::ftell(file);
	REQUIRE(size > 0);
	std::fseek(file, 0, SEEK_SET);
	std::vector<uint8_t> data(static_cast<std::size_t>(size));
	REQUIRE(std::fread(data.data(), 1, data.size(), file) == data.size());
	std::fclose(file);
	return data;
}

} // namespace

TEST_CASE("byte order conversion", "[binio]")
{
	SECTION("converting to the host order is a straight copy")
	{
		const uint8_t src[4] = { 1, 2, 3, 4 };
		uint8_t dst[4] = { 0, 0, 0, 0 };
		BinIOConvert4(BinIO_HOST_BYTE_ORDER, BinIO_HOST_BYTE_ORDER, src, dst, 1);
		REQUIRE(dst[0] == 1);
		REQUIRE(dst[1] == 2);
		REQUIRE(dst[2] == 3);
		REQUIRE(dst[3] == 4);
	}

	SECTION("converting from the host order is a straight copy")
	{
		const uint8_t src[2] = { 0xAB, 0xCD };
		uint8_t dst[2] = { 0, 0 };
		BinIOConvert2(BinIO_HOST_BYTE_ORDER, BinIO_HOST_BYTE_ORDER, src, dst, 1);
		REQUIRE(dst[0] == 0xAB);
		REQUIRE(dst[1] == 0xCD);
	}

	SECTION("big endian to host reverses the bytes")
	{
		// 0x01020304 written big endian is 01 02 03 04; read as a host (little
		// endian) int it becomes 0x04030201.
		const uint8_t src[4] = { 1, 2, 3, 4 };
		uint32_t value = 0;
		BinIOConvert4(BinIO_BIG_ENDIAN_BYTE_ORDER, BinIO_HOST_BYTE_ORDER, src, reinterpret_cast<uint8_t*>(&value), 1);

		if (BinIO_HOST_BYTE_ORDER == BinIO_LITTLE_ENDIAN_BYTE_ORDER) {
			REQUIRE(value == 0x04030201u);
		}
		else {
			REQUIRE(value == 0x01020304u);
		}
	}

	SECTION("conversion handles multiple elements")
	{
		const uint8_t src[8] = { 0, 1, 0, 1, 0, 1, 0, 1 };
		uint8_t dst[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		BinIOConvert2(BinIO_BIG_ENDIAN_BYTE_ORDER, BinIO_HOST_BYTE_ORDER, src, dst, 4);
		// Four two byte elements must all be reversed, not just the first.
		REQUIRE(dst[0] == 1);
		REQUIRE(dst[1] == 0);
		REQUIRE(dst[6] == 1);
		REQUIRE(dst[7] == 0);
	}

	SECTION("single byte conversion never changes anything")
	{
		const uint8_t src[3] = { 7, 8, 9 };
		uint8_t dst[3] = { 0, 0, 0 };
		BinIOConvert1(BinIO_BIG_ENDIAN_BYTE_ORDER, BinIO_LITTLE_ENDIAN_BYTE_ORDER, src, dst, 3);
		REQUIRE(dst[0] == 7);
		REQUIRE(dst[1] == 8);
		REQUIRE(dst[2] == 9);
	}
}

TEST_CASE("pack writes the documented byte order", "[binio]")
{
	SECTION("big endian is written most significant byte first")
	{
		unsigned char buffer[4] = { 0, 0, 0, 0 };
		spackf(buffer, "Bi", static_cast<int32_t>(0x01020304));

		REQUIRE(buffer[0] == 0x01);
		REQUIRE(buffer[1] == 0x02);
		REQUIRE(buffer[2] == 0x03);
		REQUIRE(buffer[3] == 0x04);
	}

	SECTION("host order round-trips")
	{
		unsigned char buffer[4] = { 0, 0, 0, 0 };
		spackf(buffer, "Hi", static_cast<int32_t>(0x01020304));

		int32_t value = 0;
		sunpackf(buffer, "Hi", &value);
		REQUIRE(value == 0x01020304);
	}

	SECTION("big endian round-trips")
	{
		unsigned char buffer[4] = { 0, 0, 0, 0 };
		spackf(buffer, "Bi", static_cast<int32_t>(-2));

		int32_t value = 0;
		sunpackf(buffer, "Bi", &value);
		REQUIRE(value == -2);
	}

	SECTION("ignored bytes are written but not returned")
	{
		unsigned char buffer[5] = { 0, 0, 0, 0, 0 };
		spackf(buffer, "xBi", static_cast<int32_t>(7));

		// The leading ignored byte still occupies space in the stream.
		REQUIRE(buffer[0] == 0x00);
		int32_t value = 0;
		sunpackf(buffer, "xBi", &value);
		REQUIRE(value == 7);
	}
}

TEST_CASE("tryfunpackf reports truncation instead of throwing", "[binio]")
{
	const std::string path = "lugaru-binio-try.bin";

	SECTION("a complete read succeeds")
	{
		FILE* file = std::fopen(path.c_str(), "wb");
		REQUIRE(file != nullptr);
		fpackf(file, "Bi", static_cast<int32_t>(99));
		std::fclose(file);

		file = std::fopen(path.c_str(), "rb");
		REQUIRE(file != nullptr);
		int32_t value = 0;
		REQUIRE(tryfunpackf(file, "Bi", &value));
		REQUIRE(value == 99);
		std::fclose(file);
		std::remove(path.c_str());
	}

	SECTION("a short read returns false and leaves the target untouched")
	{
		FILE* file = std::fopen(path.c_str(), "wb");
		REQUIRE(file != nullptr);
		const unsigned char partial[2] = { 0x01, 0x02 };
		REQUIRE(std::fwrite(partial, 1, sizeof(partial), file) == sizeof(partial));
		std::fclose(file);

		file = std::fopen(path.c_str(), "rb");
		REQUIRE(file != nullptr);
		int32_t value = 12345;
		REQUIRE_FALSE(tryfunpackf(file, "Bi", &value));
		REQUIRE(value == 12345);
		std::fclose(file);
		std::remove(path.c_str());
	}

	SECTION("a zero length format consumes nothing and succeeds")
	{
		FILE* file = std::fopen(path.c_str(), "wb");
		REQUIRE(file != nullptr);
		std::fclose(file);

		file = std::fopen(path.c_str(), "rb");
		REQUIRE(file != nullptr);
		REQUIRE(tryfunpackf(file, ""));
		// Reading nothing from an empty file must not consume the stream.
		int32_t value = 0;
		REQUIRE_THROWS(funpackf(file, "Bi", &value));
		std::fclose(file);
		std::remove(path.c_str());
	}

	SECTION("funpackf throws where tryfunpackf returns false")
	{
		FILE* file = std::fopen(path.c_str(), "wb");
		REQUIRE(file != nullptr);
		std::fclose(file);

		file = std::fopen(path.c_str(), "rb");
		REQUIRE(file != nullptr);
		int32_t value = 0;
		REQUIRE_THROWS_AS(funpackf(file, "Bi", &value), TruncatedFileException);
		std::fclose(file);
		std::remove(path.c_str());
	}
}

TEST_CASE("multi field records round-trip through a file", "[binio]")
{
	const std::string path = "lugaru-binio-record.bin";

	FILE* file = std::fopen(path.c_str(), "wb");
	REQUIRE(file != nullptr);
	fpackf(file, "Bi Bf Bf Bf Bl", static_cast<int32_t>(-7), 1.5f, 2.5f, 3.5f, static_cast<int64_t>(1LL << 40));
	std::fclose(file);

	file = std::fopen(path.c_str(), "rb");
	REQUIRE(file != nullptr);
	int32_t i = 0;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	int64_t l = 0;
	REQUIRE_NOTHROW(funpackf(file, "Bi Bf Bf Bf Bl", &i, &x, &y, &z, &l));
	std::fclose(file);
	std::remove(path.c_str());

	REQUIRE(i == -7);
	REQUIRE(x == 1.5f);
	REQUIRE(y == 2.5f);
	REQUIRE(z == 3.5f);
	REQUIRE(l == (1LL << 40));
}

TEST_CASE("shipped animation files are readable", "[binio]")
{
	// The animation format is the main consumer of the binary reader, and many
	// shipped files predate the trailing weapontarget block. Reading them must
	// succeed either way.
	auto readHeader = [](const char* path, int32_t& numframes, int32_t& numjoints) {
		FILE* file = std::fopen(path, "rb");
		REQUIRE(file != nullptr);
		REQUIRE_NOTHROW(funpackf(file, "Bi Bi", &numframes, &numjoints));
		std::fclose(file);
	};

	SECTION("a file that carries the trailing block reads its header")
	{
		int32_t numframes = 0;
		int32_t numjoints = 0;
		readHeader("Data/Animations/WolfIdle", numframes, numjoints);
		REQUIRE(numframes > 0);
		REQUIRE(numjoints > 0);
	}

	SECTION("a legacy file without the trailing block still reads its header")
	{
		int32_t numframes = 0;
		int32_t numjoints = 0;
		readHeader("Data/Animations/Tempanim", numframes, numjoints);
		REQUIRE(numframes > 0);
		REQUIRE(numjoints > 0);
	}

	SECTION("every shipped animation file has a plausible header")
	{
		// Guards against a data file that is empty or has a nonsense header,
		// which the unchecked reader used to accept silently.
		for (const char* path : { "Data/Animations/WolfIdle", "Data/Animations/Tempanim",
								  "Data/Animations/Run", "Data/Animations/Dead1" }) {
			const std::vector<uint8_t> data = bytesOf(path);
			REQUIRE(data.size() > 8);

			int32_t numframes = 0;
			int32_t numjoints = 0;
			readHeader(path, numframes, numjoints);
			REQUIRE(numframes > 0);
			REQUIRE(numjoints > 0);
		}
	}
}