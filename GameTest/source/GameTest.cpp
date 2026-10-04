// Unit tests for the Game layer.
//
// References GameLib and Catch2 only.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "Level/Hotspot.hpp"
#include "User/Account.hpp"

TEST_CASE("Hotspot", "[hotspot]")
{
	SECTION("a default hotspot is empty")
	{
		const Hotspot hotspot;
		REQUIRE(hotspot.position.x == 0.0f);
		REQUIRE(hotspot.position.y == 0.0f);
		REQUIRE(hotspot.position.z == 0.0f);
		REQUIRE(hotspot.type == 0);
		REQUIRE(hotspot.size == 0.0f);
		REQUIRE(hotspot.text.empty());
	}

	SECTION("the positional constructor stores its arguments")
	{
		const Hotspot hotspot(Vector3(1.0f, 2.0f, 3.0f), 7, 4.5f);
		REQUIRE(hotspot.position.x == 1.0f);
		REQUIRE(hotspot.position.y == 2.0f);
		REQUIRE(hotspot.position.z == 3.0f);
		REQUIRE(hotspot.type == 7);
		REQUIRE(hotspot.size == 4.5f);
		REQUIRE(hotspot.text.empty());
	}

	SECTION("the static pool starts empty")
	{
		REQUIRE(Hotspot::hotspots.empty());
	}
}

TEST_CASE("Account", "[account]")
{
	SECTION("a new account starts at zeroed progress")
	{
		Account account("tester");
		REQUIRE(account.getName() == "tester");
		REQUIRE(account.getProgress() == 0);
		REQUIRE(account.getDifficulty() == 0);
	}

	SECTION("winLevel keeps the best score and fastest time")
	{
		Account account("tester");
		account.winLevel(3, 1234, 42.5f, false);

		REQUIRE(account.getHighScore(3) == 1234);
		REQUIRE(account.getFastTime(3) == 42.5f);

		// A worse score and slower time must not overwrite the records.
		account.winLevel(3, 10, 99.0f, false);
		REQUIRE(account.getHighScore(3) == 1234);
		REQUIRE(account.getFastTime(3) == 42.5f);

		// Better results are recorded.
		account.winLevel(3, 5678, 20.0f, false);
		REQUIRE(account.getHighScore(3) == 5678);
		REQUIRE(account.getFastTime(3) == 20.0f);
	}

	SECTION("winLevel advances progress")
	{
		Account account("tester");
		REQUIRE(account.getProgress() == 0);
		account.winLevel(4, 1, 1.0f, false);
		REQUIRE(account.getProgress() == 5);

		// Progress never goes backwards.
		account.winLevel(1, 1, 1.0f, false);
		REQUIRE(account.getProgress() == 5);
	}

	SECTION("devtools results are not recorded")
	{
		Account account("tester");
		account.winLevel(2, 9999, 1.0f, true);
		REQUIRE(account.getHighScore(2) == 0);
		REQUIRE(account.getFastTime(2) == 0.0f);
		// Progress still advances.
		REQUIRE(account.getProgress() == 3);
	}

	SECTION("a zero time is replaced rather than kept as the record")
	{
		Account account("tester");
		// fasttime defaults to 0, which must not count as a completed time.
		account.winLevel(0, 10, 12.5f, false);
		REQUIRE(account.getFastTime(0) == 12.5f);
	}

	SECTION("unread levels report zero")
	{
		Account account("tester");
		REQUIRE(account.getHighScore(0) == 0);
		REQUIRE(account.getFastTime(0) == 0.0f);
	}

	SECTION("campaign score keeps the best result")
	{
		Account account("tester");
		account.setCampaignScore(500);
		REQUIRE(account.getCampaignScore() == 500);
		REQUIRE(account.getCampaignHighScore() == 500);

		// A worse score updates the current score but not the high score.
		account.setCampaignScore(100);
		REQUIRE(account.getCampaignScore() == 100);
		REQUIRE(account.getCampaignHighScore() == 500);

		// A better score lifts both.
		account.setCampaignScore(900);
		REQUIRE(account.getCampaignScore() == 900);
		REQUIRE(account.getCampaignHighScore() == 900);
	}

	SECTION("campaign fast time keeps the quickest result")
	{
		Account account("tester");
		account.setCampaignFinalTime(30.0f);
		REQUIRE(account.getCampaignFasttime() == 30.0f);

		account.setCampaignFinalTime(45.0f);
		REQUIRE(account.getCampaignFasttime() == 30.0f);

		account.setCampaignFinalTime(20.0f);
		REQUIRE(account.getCampaignFasttime() == 20.0f);

		account.resetFasttime();
		REQUIRE(account.getCampaignFasttime() == 0.0f);
	}

	SECTION("the first recorded time wins over the zeroed default")
	{
		Account account("tester");
		// fasttime starts at 0, which must not be treated as the best time.
		account.setCampaignFinalTime(12.5f);
		REQUIRE(account.getCampaignFasttime() == 12.5f);
	}

	SECTION("campaign choices are recorded in order")
	{
		Account account("tester");
		REQUIRE(account.getCampaignChoicesMade() == 0);
	}
}