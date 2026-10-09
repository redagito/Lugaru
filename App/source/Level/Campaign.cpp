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

#include "Level/Campaign.hpp"

#include "Game.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Utils/Folders.hpp"

#include "Utils/dirent.h"

// Global canpaign values
std::vector<CampaignLevel> campaignlevels;
std::string campaignEndText[3];

std::vector<std::string> ListCampaigns()
{
	errno = 0;
	DIR* campaigns = opendir(Folders::getResourcePath("Campaigns").c_str());
	struct dirent* campaignEntry = NULL;
	if (!campaigns) {
		perror(("Problem while loading campaigns from " + Folders::getResourcePath("Campaigns")).c_str());
		exit(EXIT_FAILURE);
	}
	std::vector<std::string> campaignNames;
	while ((campaignEntry = readdir(campaigns)) != NULL) {
		std::string name(campaignEntry->d_name);
		if (name.length() < 5) {
			continue;
		}
		if (!name.compare(name.length() - 4, 4, ".txt")) {
			campaignNames.push_back(name.substr(0, name.length() - 4));
		}
	}
	closedir(campaigns);
	return campaignNames;
}

void LoadCampaign(GameState& gamestate, GameAssets& assets)
{
	if (!Account::hasActive()) {
		return;
	}
	std::ifstream ipstream(Folders::getResourcePath("Campaigns/" + Account::active().getCurrentCampaign() + ".txt"));
	if (!ipstream.good()) {
		if (Account::active().getCurrentCampaign() == "main") {
			std::cerr << "Could not find main campaign!" << std::endl;
			return;
		}
		std::cerr << "Could not find campaign \"" << Account::active().getCurrentCampaign() << "\", falling back to main." << std::endl;
		Account::active().setCurrentCampaign("main");
		return LoadCampaign(gamestate, assets);
	}
	ipstream.ignore(256, ':');
	int numlevels;
	ipstream >> numlevels;
	campaignlevels.clear();
	for (int i = 0; i < numlevels; i++) {
		CampaignLevel cl;
		ipstream >> cl;
		campaignlevels.push_back(cl);
	}
	campaignEndText[0] = "Congratulations!";
	campaignEndText[1] = std::string("You have completed ") + Account::active().getCurrentCampaign() + " campaign";
	campaignEndText[2] = "and restored peace to the island of Lugaru.";
	if (ipstream.good()) {
		ipstream.ignore(256, ':');
		getline(ipstream, campaignEndText[0]);
		getline(ipstream, campaignEndText[1]);
		getline(ipstream, campaignEndText[2]);
	}
	ipstream.close();

	std::ifstream test(Folders::getResourcePath("Textures/" + Account::active().getCurrentCampaign() + "/World.png"));
	if (test.good()) {
		assets.Mainmenuitems[7].load("Textures/" + Account::active().getCurrentCampaign() + "/World.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	}
	else {
		assets.Mainmenuitems[7].load("Textures/World.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	}

	if (Account::active().getCampaignChoicesMade() == 0) {
		Account::active().setCampaignScore(0);
		Account::active().resetFasttime();
	}
}

CampaignLevel::CampaignLevel()
	: width(10)
	, choosenext(1)
{
	location.x = 0;
	location.y = 0;
}

int CampaignLevel::getStartX()
{
	return 30 + 120 + location.x * 400 / 512;
}

int CampaignLevel::getStartY()
{
	return 30 + 30 + (512 - location.y) * 400 / 512;
}

int CampaignLevel::getEndX()
{
	return getStartX() + width;
}

int CampaignLevel::getEndY()
{
	return getStartY() + width;
}

Vector3 CampaignLevel::getCenter()
{
	Vector3 center;
	center.x = getStartX() + width / 2;
	center.y = getStartY() + width / 2;
	return center;
}

int CampaignLevel::getWidth()
{
	return width;
}

std::istream& CampaignLevel::operator<<(std::istream& is)
{
	is.ignore(256, ':');
	is.ignore(256, ':');
	is.ignore(256, ' ');
	is >> mapname;
	is.ignore(256, ':');
	is >> description;
	for (size_t pos = description.find('_'); pos != std::string::npos; pos = description.find('_', pos)) {
		description.replace(pos, 1, 1, ' ');
	}
	is.ignore(256, ':');
	is >> choosenext;
	is.ignore(256, ':');
	int numnext, next;
	is >> numnext;
	for (int j = 0; j < numnext; j++) {
		is.ignore(256, ':');
		is >> next;
		nextlevel.push_back(next - 1);
	}
	is.ignore(256, ':');
	is >> location.x;
	is.ignore(256, ':');
	is >> location.y;
	return is;
}
