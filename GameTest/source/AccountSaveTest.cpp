// Unit tests for the Game layer.
//
// References GameLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <string>
#include <vector>

#include "User/Account.hpp"
#include "Utils/Folders.hpp"

namespace
{

// Account keeps its account list in a private static, so save/load tests own
// that global state. Each case writes and reads back its own file.
const char* kSavePath = "lugaru-account-roundtrip.bin";

} // namespace

TEST_CASE("Account difficulty is settable", "[account]")
{
	Account account("tester");
	REQUIRE(account.getDifficulty() == 0);

	account.setDifficulty(2);
	REQUIRE(account.getDifficulty() == 2);

	account.setDifficulty(0);
	REQUIRE(account.getDifficulty() == 0);
}

TEST_CASE("Account campaign selection", "[account]")
{
	Account account("tester");

	SECTION("the default campaign is main")
	{
		REQUIRE(account.getCurrentCampaign() == "main");
	}

	SECTION("progress is tracked per campaign")
	{
		account.setCurrentCampaign("main");
		account.setCampaignScore(250);
		REQUIRE(account.getCampaignScore() == 250);

		account.setCurrentCampaign("empire");
		// A different campaign starts from scratch.
		REQUIRE(account.getCampaignScore() == 0);
		REQUIRE(account.getCampaignHighScore() == 0);
		REQUIRE(account.getCurrentCampaign() == "empire");

		account.setCampaignScore(75);
		REQUIRE(account.getCampaignScore() == 75);

		// The original campaign is untouched.
		account.setCurrentCampaign("main");
		REQUIRE(account.getCampaignScore() == 250);
	}
}

TEST_CASE("Account save and load round-trip", "[account]")
{
	SECTION("progress survives a write and read")
	{
		// Build a populated account.
		Account original("RoundTrip");
		original.setDifficulty(3);
		original.winLevel(2, 4321, 12.5f, false);
		original.winLevel(7, 99, 3.25f, false);
		original.setCampaignScore(7777);
		original.setCampaignFinalTime(41.5f);

		// Write it out through the public API.
		Account::add(original.getName());
		Account& stored = Account::get(Account::getNbAccounts() - 1);
		stored.setDifficulty(original.getDifficulty());
		stored.winLevel(2, 4321, 12.5f, false);
		stored.winLevel(7, 99, 3.25f, false);
		stored.setCampaignScore(7777);
		stored.setCampaignFinalTime(41.5f);

		Account::saveFile(kSavePath);
		REQUIRE(Folders::file_exists(kSavePath));

		// Read it back into a fresh set of accounts.
		Account::loadFile(kSavePath);
		REQUIRE(Account::getNbAccounts() >= 1);

		Account& loaded = Account::get(Account::getNbAccounts() - 1);
		REQUIRE(loaded.getName() == "RoundTrip");
		REQUIRE(loaded.getDifficulty() == 3);
		REQUIRE(loaded.getProgress() == 8);
		REQUIRE(loaded.getHighScore(2) == 4321);
		REQUIRE(loaded.getFastTime(2) == 12.5f);
		REQUIRE(loaded.getHighScore(7) == 99);
		REQUIRE(loaded.getFastTime(7) == 3.25f);
		REQUIRE(loaded.getCampaignScore() == 7777);
		REQUIRE(loaded.getCampaignHighScore() == 7777);
		REQUIRE(loaded.getCampaignFasttime() == 41.5f);

		std::remove(kSavePath);
	}

	SECTION("an empty account still round-trips")
	{
		Account::add("Empty");

		Account::saveFile(kSavePath);
		Account::loadFile(kSavePath);

		Account& loaded = Account::get(Account::getNbAccounts() - 1);
		REQUIRE(loaded.getName() == "Empty");
		REQUIRE(loaded.getProgress() == 0);

		std::remove(kSavePath);
	}

	SECTION("a truncated save file is reported rather than silently accepted")
	{
		Account::add("Truncated");
		Account::saveFile(kSavePath);
		REQUIRE(Folders::file_exists(kSavePath));

		// Keep only the leading half of the file, simulating a partial write.
		std::vector<unsigned char> contents;
		{
			FILE* file = std::fopen(kSavePath, "rb");
			REQUIRE(file != nullptr);
			std::fseek(file, 0, SEEK_END);
			const long size = std::ftell(file);
			REQUIRE(size > 8);
			std::fseek(file, 0, SEEK_SET);
			contents.resize(static_cast<std::size_t>(size));
			REQUIRE(std::fread(contents.data(), 1, contents.size(), file) == contents.size());
			std::fclose(file);
		}

		const std::size_t keep = contents.size() / 2;
		{
			FILE* file = std::fopen(kSavePath, "wb");
			REQUIRE(file != nullptr);
			REQUIRE(std::fwrite(contents.data(), 1, keep, file) == keep);
			std::fclose(file);
		}

		// The strict reader must refuse to invent the missing records.
		REQUIRE_THROWS(Account::loadFile(kSavePath));

		// ...and must not leave a half populated account list behind.
		REQUIRE(Account::getNbAccounts() == 0);
		REQUIRE_FALSE(Account::hasActive());

		std::remove(kSavePath);
	}
}

TEST_CASE("Account file loading is defensive", "[account]")
{
	SECTION("loading a missing file deactivates every account")
	{
		// Account keeps its list in a private static with no reset API, so this
		// asserts on the active flag rather than the list size.
		Account::add("Someone");
		Account::setActive(Account::getNbAccounts() - 1);
		REQUIRE(Account::hasActive());

		Account::loadFile("lugaru-no-such-users-file.bin");
		REQUIRE_FALSE(Account::hasActive());
	}

	SECTION("an account with an empty name is given a default on load")
	{
		// The fallback lives in the file constructor, so it only applies once an
		// account has been through a save/load cycle.
		Account::add("");
		Account& stored = Account::get(Account::getNbAccounts() - 1);
		REQUIRE(stored.getName().empty());

		Account::saveFile("lugaru-empty-name.bin");
		Account::loadFile("lugaru-empty-name.bin");
		REQUIRE(Account::getNbAccounts() >= 1);
		REQUIRE(Account::get(Account::getNbAccounts() - 1).getName() == "Lugaru Player");
		std::remove("lugaru-empty-name.bin");
	}
}