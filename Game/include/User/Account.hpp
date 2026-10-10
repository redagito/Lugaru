/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ACCOUNT_HPP_
#define _ACCOUNT_HPP_

#include <map>
#include <string>
#include <vector>
#include <cstdio>

struct CampaignProgress
{
	int highscore = 0;
	float fasttime = 0.f;
	int score = 0;
	float time = 0.f;
	std::vector<int> choices;
};

class Account
{
public:
	static void destroyActive();
	static void setActive(int i);
	static void add(const std::string& name);
	static Account& get(int i);
	static void loadFile(std::string filename);
	static void saveFile(std::string filename);
	static int getNbAccounts();

	static bool hasActive();
	static Account& active();

	Account(const std::string& name = "");
	Account(FILE* tfile);

	void endGame();
	void winCampaignLevel(int choice, int score, float time);
	void winLevel(int level, int score, float time, bool devtools);

	// getter and setters
	int getDifficulty();
	void setDifficulty(int i);
	const std::string& getName();
	int getCampaignScore();
	int getCampaignChoicesMade();

	int getCampaignChoice(int i);

	void setCampaignScore(int s);

	void setCampaignFinalTime(float t);

	float getCampaignFasttime();
	void resetFasttime();
	int getCampaignHighScore();
	int getHighScore(int i);
	float getFastTime(int i);
	int getProgress();
	std::string getCurrentCampaign();
	void setCurrentCampaign(const std::string& name);

private:
	//statics
	static std::vector<Account> accounts;
	static int i_active;

	void save(FILE* tfile);

	int difficulty;
	int progress; // progress in challenge levels
	float points;
	int highscore[50];
	float fasttime[50];
	bool unlocked[60];
	std::string name;

	std::string currentCampaign;
	std::map<std::string, CampaignProgress> campaignProgress;
};

#endif
