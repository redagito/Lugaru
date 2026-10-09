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

#include "Devtools/ConsoleCmds.hpp"
#include "GameGlobals.h"

#include "Audio/AudioState.hpp"
#include "Game.hpp"
#include "Console.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Level/Dialog.hpp"
#include "Level/Hotspot.hpp"
#include "Tutorial.hpp"
#include "Utils/Folders.hpp"

#include "Globals.h"

#include <json/value.h>
#include <json/writer.h>

const char* cmd_names[cmd_count] = {
#define DECLARE_COMMAND(cmd) #cmd,
#include "Devtools/ConsoleCmds.inc"
#undef DECLARE_COMMAND
};

console_handler cmd_handlers[cmd_count] = {
#define DECLARE_COMMAND(cmd) ch_##cmd,
#include "Devtools/ConsoleCmds.inc"
#undef DECLARE_COMMAND
};

#ifdef _MSC_VER

#define strncasecmp(x,y,z) _strnicmp(x,y,z)

#endif

using namespace Game;

/* globals */

extern bool campaign;

/* defined in GameTick.cpp */

extern int whichlevel;

float tintr = 1, tintg = 1, tintb = 1;

/* Helpers used in console commands */

/* Return true if PFX is a prefix of STR (case-insensitive).  */
static bool stripfx(const char* str, const char* pfx)
{
	return !strncasecmp(str, pfx, strlen(pfx));
}

static void set_proportion(int pnum, const char* args)
{
	float headprop, bodyprop, armprop, legprop;

	sscanf(args, "%f%f%f%f", &headprop, &bodyprop, &armprop, &legprop);

	Person::players[pnum]->setProportions(headprop, bodyprop, armprop, legprop);
}

static void set_protection(int pnum, const char* args)
{
	float head, high, low;
	sscanf(args, "%f%f%f", &head, &high, &low);

	Person::players[pnum]->protectionhead = head;
	Person::players[pnum]->protectionhigh = high;
	Person::players[pnum]->protectionlow = low;
}

static void set_armor(int pnum, const char* args)
{
	float head, high, low;
	sscanf(args, "%f%f%f", &head, &high, &low);

	Person::players[pnum]->armorhead = head;
	Person::players[pnum]->armorhigh = high;
	Person::players[pnum]->armorlow = low;
}

static void set_metal(int pnum, const char* args)
{
	float head, high, low;
	sscanf(args, "%f%f%f", &head, &high, &low);

	Person::players[pnum]->metalhead = head;
	Person::players[pnum]->metalhigh = high;
	Person::players[pnum]->metallow = low;
}

static void set_noclothes(int pnum, const char*, GameState& gamestate, GameAssets& assets)
{
	Person::players[pnum]->clothes.clear();
	Person::players[pnum]->clothestintr.clear();
	Person::players[pnum]->clothestintg.clear();
	Person::players[pnum]->clothestintb.clear();
	Person::players[pnum]->skeleton.drawmodel.textureptr.load(
		PersonType::types[Person::players[pnum]->creature].skins[Person::players[pnum]->whichskin], 1,
		&Person::players[pnum]->skeleton.skinText[0], &Person::players[pnum]->skeleton.skinsize, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
}

static void set_clothes(int pnum, const char* args, GameState& gamestate, GameAssets& assets, Console& console)
{
	char buf[64];
	snprintf(buf, 63, "Textures/%s.png", args);

	const std::string file_path = Folders::getResourcePath(buf);
	FILE* tfile = nullptr;
	tfile = fopen(file_path.c_str(), "rb");
	if (tfile == NULL) {
		perror((std::string("Couldn't find file ") + file_path + " to assign as clothes").c_str());

		console.push(std::string("Could not load the requested texture '") + args + "', aborting.");

		return;
	}
	fclose(tfile);

	int id = (int)Person::players[pnum]->clothes.size();
	Person::players[pnum]->clothes.push_back(std::string(buf));
	Person::players[pnum]->clothestintr.push_back(tintr);
	Person::players[pnum]->clothestintg.push_back(tintg);
	Person::players[pnum]->clothestintb.push_back(tintb);

	if (!Person::players[pnum]->addClothes(id, gamestate, assets)) {
		return;
	}

	Person::players[pnum]->DoMipmaps();
}

static void list_clothes(int pnum)
{
	printf("Clothes from player %d:\n", pnum);
	for (unsigned i = 0; i < Person::players[pnum]->clothes.size(); i++) {
		printf("%s (%f %f %f)\n",
			Person::players[pnum]->clothes[i].c_str(),
			Person::players[pnum]->clothestintr[i],
			Person::players[pnum]->clothestintg[i],
			Person::players[pnum]->clothestintb[i]);
	}
}

/* Console commands themselves */

void ch_quit(const char*, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.tryquit = 1;
}

void ch_map(const char* args, GameState& gamestate, GameAssets& assets, Console& console)
{
	if (!LoadLevel(args, false, gamestate, assets, console)) {
		console.push(std::string("Could not load the requested level '") + args + "', aborting.");
	}
	whichlevel = -2;
	campaign = 0;
}

void ch_save_json(const char* args, GameState& gamestate, GameAssets& assets, Console&)
{
	std::string map_path = Folders::getUserDataPath() + "/Maps";
	Folders::makeDirectory(map_path);
	map_path = map_path + "/" + args + ".json";

	std::ofstream map_file(map_path);
	if (map_file.fail()) {
		perror((std::string("Couldn't open file ") + map_path + " for saving").c_str());
		return;
	}
	else {
		std::cout << "saving in " << map_path << std::endl;
	}

	Json::Value map_data;

	map_data["version"] = 13;

	map_data["map"]["type"] = gamestate.maptype;
	map_data["map"]["hostile"] = gamestate.hostile;
	map_data["map"]["viewdistance"] = gamestate.viewdistance;
	map_data["map"]["fadestart"] = gamestate.fadestart;

	map_data["map"]["skybox"]["texture"] = gamestate.skyboxtexture;
	map_data["map"]["skybox"]["r"] = gamestate.skyboxr;
	map_data["map"]["skybox"]["g"] = gamestate.skyboxg;
	map_data["map"]["skybox"]["b"] = gamestate.skyboxb;
	map_data["map"]["skybox"]["lightr"] = gamestate.skyboxlightr;
	map_data["map"]["skybox"]["lightg"] = gamestate.skyboxlightg;
	map_data["map"]["skybox"]["lightb"] = gamestate.skyboxlightb;

	map_data["map"]["dialogs"] = Dialog::saveDialogs();

	map_data["map"]["environment"] = gamestate.environment;

	for (unsigned int k = 0; k < Object::objects.size(); k++) {
		map_data["map"]["objects"][k] = *Object::objects[k];
	}

	for (unsigned i = 0; i < Hotspot::hotspots.size(); i++) {
		map_data["map"]["hotspots"][i]["type"] = Hotspot::hotspots[i].type;
		map_data["map"]["hotspots"][i]["size"] = Hotspot::hotspots[i].size;
		map_data["map"]["hotspots"][i]["text"] = Hotspot::hotspots[i].text;
		map_data["map"]["hotspots"][i]["position"] = Hotspot::hotspots[i].position;
	}

	if (Person::players.size() > maxplayers) {
		std::cout << "Warning: this level contains more players than allowed" << std::endl;
	}
	for (unsigned j = 0; j < Person::players.size(); j++) {
		map_data["map"]["players"][j] = Person::players[j]->save(assets);
	}

	for (int j = 0; j < gamestate.numpathpoints; j++) {
		map_data["map"]["pathpoints"][j]["pos"] = gamestate.pathpoint[j];
		for (int k = 0; k < gamestate.numpathpointconnect[j]; k++) {
			map_data["map"]["pathpoints"][j]["connect"][k] = gamestate.pathpointconnect[j][k];
		}
	}

	map_data["map"]["center"] = gamestate.mapcenter;
	map_data["map"]["radius"] = gamestate.mapradius;

	Json::StreamWriterBuilder builder;
	// default is "All", this allows to print arrays as one line
	builder.settings_["commentStyle"] = "None";
	builder.settings_["precision"] = 7;
	map_file << Json::writeString(builder, map_data);

	map_file.close();
}

void ch_convert_to_json(const char* args, GameState& gamestate, GameAssets& assets, Console& console)
{
	ch_map(args, gamestate, assets, console);
	ch_save_json(args, gamestate, assets, console);
}

void ch_save(const char* args, GameState& gamestate, GameAssets& assets, Console&)
{
	std::string map_path = Folders::getUserDataPath() + "/Maps";
	Folders::makeDirectory(map_path);
	map_path = map_path + "/" + args;

	int mapvers = 12;

	FILE* tfile;
	tfile = fopen(map_path.c_str(), "wb");
	if (tfile == NULL) {
		perror((std::string("Couldn't open file ") + map_path + " for saving").c_str());
		return;
	}
	fpackf(tfile, "Bi", mapvers);
	fpackf(tfile, "Bi", gamestate.maptype);
	fpackf(tfile, "Bi", gamestate.hostile);
	fpackf(tfile, "Bf Bf", gamestate.viewdistance, gamestate.fadestart);
	fpackf(tfile, "Bb Bf Bf Bf", gamestate.skyboxtexture, gamestate.skyboxr, gamestate.skyboxg, gamestate.skyboxb);
	fpackf(tfile, "Bf Bf Bf", gamestate.skyboxlightr, gamestate.skyboxlightg, gamestate.skyboxlightb);
	fpackf(tfile, "Bf Bf Bf Bf Bf Bi", Person::players[0]->coords.x, Person::players[0]->coords.y, Person::players[0]->coords.z,
		Person::players[0]->yaw, Person::players[0]->targetyaw, Person::players[0]->num_weapons);
	if (Person::players[0]->num_weapons > 0 && Person::players[0]->num_weapons < 5) {
		for (int j = 0; j < Person::players[0]->num_weapons; j++) {
			fpackf(tfile, "Bi", assets.weapons.weapons[Person::players[0]->weaponids[j]].getType());
		}
	}

	fpackf(tfile, "Bf Bf Bf", Person::players[0]->armorhead, Person::players[0]->armorhigh, Person::players[0]->armorlow);
	fpackf(tfile, "Bf Bf Bf", Person::players[0]->protectionhead, Person::players[0]->protectionhigh, Person::players[0]->protectionlow);
	fpackf(tfile, "Bf Bf Bf", Person::players[0]->metalhead, Person::players[0]->metalhigh, Person::players[0]->metallow);
	fpackf(tfile, "Bf Bf", Person::players[0]->power, Person::players[0]->speedmult);

	fpackf(tfile, "Bi", Person::players[0]->clothes.size());

	fpackf(tfile, "Bi Bi", Person::players[0]->whichskin, Person::players[0]->creature);

	Dialog::saveDialogs(tfile);

	for (unsigned k = 0; k < Person::players[0]->clothes.size(); k++) {
		int templength = (int)Person::players[0]->clothes[k].size();
		fpackf(tfile, "Bi", templength);
		for (int l = 0; l < templength; l++) {
			fpackf(tfile, "Bb", Person::players[0]->clothes[k][l]);
		}
		fpackf(tfile, "Bf Bf Bf", Person::players[0]->clothestintr[k], Person::players[0]->clothestintg[k], Person::players[0]->clothestintb[k]);
	}

	fpackf(tfile, "Bi", gamestate.environment);

	fpackf(tfile, "Bi", Object::objects.size());

	for (unsigned int k = 0; k < Object::objects.size(); k++) {
		fpackf(tfile, "Bi Bf Bf Bf Bf Bf Bf", Object::objects[k]->type, Object::objects[k]->yaw, Object::objects[k]->pitch,
			Object::objects[k]->position.x, Object::objects[k]->position.y, Object::objects[k]->position.z, Object::objects[k]->scale);
	}

	fpackf(tfile, "Bi", Hotspot::hotspots.size());
	for (unsigned i = 0; i < Hotspot::hotspots.size(); i++) {
		fpackf(tfile, "Bi Bf Bf Bf Bf", Hotspot::hotspots[i].type, Hotspot::hotspots[i].size, Hotspot::hotspots[i].position.x, Hotspot::hotspots[i].position.y, Hotspot::hotspots[i].position.z);
		int templength = (int)Hotspot::hotspots[i].text.size();
		fpackf(tfile, "Bi", templength);
		for (int l = 0; l < templength; l++) {
			fpackf(tfile, "Bb", Hotspot::hotspots[i].text[l]);
		}
	}

	fpackf(tfile, "Bi", Person::players.size());
	if (Person::players.size() > maxplayers) {
		std::cout << "Warning: this level contains more players than allowed" << std::endl;
	}
	for (unsigned j = 1; j < Person::players.size(); j++) {
		fpackf(tfile, "Bi Bi Bf Bf Bf Bi Bi Bf Bb Bf", Person::players[j]->whichskin, Person::players[j]->creature,
			Person::players[j]->coords.x, Person::players[j]->coords.y, Person::players[j]->coords.z,
			Person::players[j]->num_weapons, Person::players[j]->howactive, Person::players[j]->scale, Person::players[j]->immobile, Person::players[j]->yaw);
		if (Person::players[j]->num_weapons < 5) {
			for (int k = 0; k < Person::players[j]->num_weapons; k++) {
				fpackf(tfile, "Bi", assets.weapons.weapons[Person::players[j]->weaponids[k]].getType());
			}
		}
		if (Person::players[j]->numwaypoints < 30) {
			fpackf(tfile, "Bi", Person::players[j]->numwaypoints);
			for (int k = 0; k < Person::players[j]->numwaypoints; k++) {
				fpackf(tfile, "Bf", Person::players[j]->waypoints[k].x);
				fpackf(tfile, "Bf", Person::players[j]->waypoints[k].y);
				fpackf(tfile, "Bf", Person::players[j]->waypoints[k].z);
				fpackf(tfile, "Bi", Person::players[j]->waypointtype[k]);
			}
			fpackf(tfile, "Bi", Person::players[j]->waypoint);
		}
		else {
			Person::players[j]->numwaypoints = 0;
			Person::players[j]->waypoint = 0;
			fpackf(tfile, "Bi Bi Bi", Person::players[j]->numwaypoints, Person::players[j]->waypoint, Person::players[j]->waypoint);
		}

		fpackf(tfile, "Bf Bf Bf", Person::players[j]->armorhead, Person::players[j]->armorhigh, Person::players[j]->armorlow);
		fpackf(tfile, "Bf Bf Bf", Person::players[j]->protectionhead, Person::players[j]->protectionhigh, Person::players[j]->protectionlow);
		fpackf(tfile, "Bf Bf Bf", Person::players[j]->metalhead, Person::players[j]->metalhigh, Person::players[j]->metallow);
		fpackf(tfile, "Bf Bf", Person::players[j]->power, Person::players[j]->speedmult);
		fpackf(tfile, "Bf Bf Bf Bf", Person::players[j]->getProportion(0), Person::players[j]->getProportion(1), Person::players[j]->getProportion(2), Person::players[j]->getProportion(3));

		fpackf(tfile, "Bi", Person::players[j]->clothes.size());
		for (unsigned k = 0; k < Person::players[j]->clothes.size(); k++) {
			int templength = (int)Person::players[j]->clothes[k].size();
			fpackf(tfile, "Bi", templength);
			for (int l = 0; l < templength; l++) {
				fpackf(tfile, "Bb", Person::players[j]->clothes[k][l]);
			}
			fpackf(tfile, "Bf Bf Bf", Person::players[j]->clothestintr[k], Person::players[j]->clothestintg[k], Person::players[j]->clothestintb[k]);
		}
	}

	fpackf(tfile, "Bi", gamestate.numpathpoints);
	for (int j = 0; j < gamestate.numpathpoints; j++) {
		fpackf(tfile, "Bf Bf Bf Bi", gamestate.pathpoint[j].x, gamestate.pathpoint[j].y, gamestate.pathpoint[j].z, gamestate.numpathpointconnect[j]);
		for (int k = 0; k < gamestate.numpathpointconnect[j]; k++) {
			fpackf(tfile, "Bi", gamestate.pathpointconnect[j][k]);
		}
	}

	fpackf(tfile, "Bf Bf Bf Bf", gamestate.mapcenter.x, gamestate.mapcenter.y, gamestate.mapcenter.z, gamestate.mapradius);

	fclose(tfile);
}

void ch_tint(const char* args, GameState&, GameAssets&, Console&)
{
	sscanf(args, "%f%f%f", &tintr, &tintg, &tintb);
}

void ch_tintr(const char* args, GameState&, GameAssets&, Console&)
{
	tintr = atof(args);
}

void ch_tintg(const char* args, GameState&, GameAssets&, Console&)
{
	tintg = atof(args);
}

void ch_tintb(const char* args, GameState&, GameAssets&, Console&)
{
	tintb = atof(args);
}

void ch_speed(const char* args, GameState&, GameAssets&, Console&)
{
	Person::players[0]->speedmult = atof(args);
}

void ch_strength(const char* args, GameState&, GameAssets&, Console&)
{
	Person::players[0]->power = atof(args);
}

void ch_power(const char* args, GameState&, GameAssets&, Console&)
{
	Person::players[0]->power = atof(args);
}

void ch_size(const char* args, GameState&, GameAssets&, Console&)
{
	Person::players[0]->scale = atof(args) * .2;
}

void ch_sizenear(const char* args, GameState&, GameAssets&, Console&)
{
	int closest = findClosestPlayer();
	if (closest >= 0) {
		Person::players[closest]->scale = atof(args) * .2;
	}
}

void ch_proportion(const char* args, GameState&, GameAssets&, Console&)
{
	set_proportion(0, args);
}

void ch_proportionnear(const char* args, GameState&, GameAssets&, Console&)
{
	int closest = findClosestPlayer();
	if (closest >= 0) {
		set_proportion(closest, args);
	}
}

void ch_protection(const char* args, GameState&, GameAssets&, Console&)
{
	set_protection(0, args);
}

void ch_protectionnear(const char* args, GameState&, GameAssets&, Console&)
{
	int closest = findClosestPlayer();
	if (closest >= 0) {
		set_protection(closest, args);
	}
}

void ch_armor(const char* args, GameState&, GameAssets&, Console&)
{
	set_armor(0, args);
}

void ch_armornear(const char* args, GameState&, GameAssets&, Console&)
{
	int closest = findClosestPlayer();
	if (closest >= 0) {
		set_armor(closest, args);
	}
}

void ch_protectionreset(const char*, GameState&, GameAssets&, Console&)
{
	set_protection(0, "1 1 1");
	set_armor(0, "1 1 1");
}

void ch_metal(const char* args, GameState&, GameAssets&, Console&)
{
	set_metal(0, args);
}

void ch_noclothes(const char* args, GameState& gamestate, GameAssets& assets, Console&)
{
	set_noclothes(0, args, gamestate, assets);
}

void ch_noclothesnear(const char* args, GameState& gamestate, GameAssets& assets, Console&)
{
	int closest = findClosestPlayer();
	if (closest >= 0) {
		set_noclothes(closest, args, gamestate, assets);
	}
}

void ch_clothes(const char* args, GameState& gamestate, GameAssets& assets, Console& console)
{
	set_clothes(0, args, gamestate, assets, console);
}

void ch_clothesnear(const char* args, GameState& gamestate, GameAssets& assets, Console& console)
{
	int closest = findClosestPlayer();
	if (closest >= 0) {
		set_clothes(closest, args, gamestate, assets, console);
	}
}

void ch_clotheslist(const char*, GameState&, GameAssets&, Console&)
{
	list_clothes(0);
}

void ch_clotheslistnear(const char*, GameState&, GameAssets&, Console&)
{
	int closest = findClosestPlayer();
	if (closest >= 0) {
		list_clothes(closest);
	}
}

void ch_belt(const char*, GameState&, GameAssets&, Console&)
{
	Person::players[0]->skeleton.clothes = !Person::players[0]->skeleton.clothes;
}

void ch_cellophane(const char*, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.cellophane = !gamestate.cellophane;
}

void ch_funnybunny(const char*, GameState& gamestate, GameAssets& assets, Console&)
{
	Person::players[0]->changeCreatureType(rabbittype, Tutorial::active, gamestate, assets);
	Person::players[0]->headless = 0;
	set_proportion(0, "1 1 1 1");
}

void ch_wolfie(const char*, GameState& gamestate, GameAssets& assets, Console&)
{
	Person::players[0]->changeCreatureType(wolftype, Tutorial::active, gamestate, assets);
	set_proportion(0, "1 1 1 1");
}

void ch_lizardwolf(const char*, GameState& gamestate, GameAssets& assets, Console&)
{
	Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/FurWolfLizard.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
}

void ch_darko(const char*, GameState& gamestate, GameAssets& assets, Console&)
{
	Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/FurDarko.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
}

void ch_sizemin(const char*, GameState&, GameAssets&, Console&)
{
	for (unsigned i = 1; i < Person::players.size(); i++) {
		if (Person::players[i]->scale < 0.8 * 0.2) {
			Person::players[i]->scale = 0.8 * 0.2;
		}
	}
}

void ch_tutorial(const char* args, GameState&, GameAssets&, Console&)
{
	Tutorial::active = atoi(args);
}

void ch_hostile(const char* args, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.hostile = atoi(args);
}

void ch_type(const char* args, GameState& gamestate, GameAssets&, Console&)
{
	int n = sizeof(editortypenames) / sizeof(editortypenames[0]);
	for (int i = 0; i < n; i++) {
		if (stripfx(args, editortypenames[i])) {
			gamestate.editoractive = i;
			break;
		}
	}
}

void ch_path(const char* args, GameState& gamestate, GameAssets&, Console&)
{
	unsigned int n = sizeof(pathtypenames) / sizeof(pathtypenames[0]);
	for (unsigned int i = 0; i < n; i++) {
		if (stripfx(args, pathtypenames[i])) {
			gamestate.editorpathtype = i;
			break;
		}
	}
}

void ch_hs(const char* args, GameState&, GameAssets&, Console&)
{
	float size;
	int type, shift;
	sscanf(args, "%f%d %n", &size, &type, &shift);

	Hotspot::hotspots.emplace_back(Person::players[0]->coords, type, size);

	Hotspot::hotspots.back().text = std::string(args + shift);
}

void ch_dialog(const char* args, GameState&, GameAssets&, Console&)
{
	int type;
	char buf1[32];

	sscanf(args, "%d %31s", &type, buf1);
	std::string filename = std::string("Dialogues/") + buf1 + ".txt";

	Dialog::dialogs.push_back(Dialog(type, filename));

	Dialog::directing = true;
	Dialog::indialogue = 0;
	Dialog::whichdialogue = Dialog::dialogs.size();
}

void ch_fixdialog(const char* args, GameState&, GameAssets&, Console&)
{
	char buf1[32];
	int whichdlg = 0;

	sscanf(args, "%d %31s", &whichdlg, buf1);
	std::string filename = std::string("Dialogues/") + buf1 + ".txt";

	Dialog::dialogs[whichdlg] = Dialog(Dialog::dialogs[whichdlg].type, filename);
}

void ch_fixtype(const char* args, GameState&, GameAssets&, Console&)
{
	int whichdlg = 0;
	int type = 0;
	sscanf(args, "%d %d", &whichdlg, &type);
	Dialog::dialogs[whichdlg].type = type;
}

void ch_fixrotation(const char*, GameState&, GameAssets&, Console&)
{
	int playerId = Dialog::currentScene().participantfocus;
	Dialog::currentDialog().participantyaw[playerId] = Person::players[playerId]->yaw;
}

void ch_ddialog(const char* args, GameState&, GameAssets&, Console&)
{
	if (Dialog::dialogs.empty() || Dialog::inDialog()) {
		return;
	}

	int dlg = -1;
	sscanf(args, "%d", &dlg);
	if (dlg == -1) {
		// Remove last entry
		Dialog::dialogs.pop_back();
		return;
	}

	if (dlg >= int(Dialog::dialogs.size())) {
		// Invalid index, abort
		return;
	}

	// Erase given index, higher indexes will be decreased by 1
	Dialog::dialogs.erase(Dialog::dialogs.begin() + dlg);
}

void ch_dhs(const char*, GameState&, GameAssets&, Console&)
{
	if (!Hotspot::hotspots.empty()) {
		Hotspot::hotspots.pop_back();
	}
}

void ch_immobile(const char*, GameState&, GameAssets&, Console&)
{
	Person::players[0]->immobile = 1;
}

void ch_allimmobile(const char*, GameState&, GameAssets&, Console&)
{
	for (unsigned i = 1; i < Person::players.size(); i++) {
		Person::players[i]->immobile = 1;
	}
}

void ch_mobile(const char*, GameState&, GameAssets&, Console&)
{
	Person::players[0]->immobile = 0;
}

void ch_default(const char*, GameState& gamestate, GameAssets& assets, Console&)
{
	Person::players[0]->armorhead = 1;
	Person::players[0]->armorhigh = 1;
	Person::players[0]->armorlow = 1;
	Person::players[0]->protectionhead = 1;
	Person::players[0]->protectionhigh = 1;
	Person::players[0]->protectionlow = 1;
	Person::players[0]->metalhead = 1;
	Person::players[0]->metalhigh = 1;
	Person::players[0]->metallow = 1;
	Person::players[0]->power = 1;
	Person::players[0]->speedmult = 1;
	Person::players[0]->scale = PersonType::types[Person::players[0]->creature].defaultScale;

	Person::players[0]->setProportions(1, 1, 1, 1);

	Person::players[0]->clothes.clear();
	Person::players[0]->clothestintr.clear();
	Person::players[0]->clothestintg.clear();
	Person::players[0]->clothestintb.clear();
	Person::players[0]->skeleton.drawmodel.textureptr.load(
		PersonType::types[Person::players[0]->creature].skins[Person::players[0]->whichskin], 1,
		&Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });

	gamestate.editoractive = typeactive;
	Person::players[0]->immobile = 0;
}

void ch_play(const char* args, GameState& gamestate, GameAssets& assets, Console&)
{
	int dlg;
	sscanf(args, "%d", &dlg);
	Dialog::whichdialogue = dlg;

	if (Dialog::whichdialogue >= int(Dialog::dialogs.size())) {
		return;
	}

	Dialog::currentDialog().play([&gamestate]() { Game::playdialoguescenesound(gamestate); }, assets);
}

void ch_mapkilleveryone(const char*, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.maptype = mapkilleveryone;
}

void ch_mapkillmost(const char*, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.maptype = mapkillmost;
}

void ch_mapkillsomeone(const char*, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.maptype = mapkillsomeone;
}

void ch_mapgosomewhere(const char*, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.maptype = mapgosomewhere;
}

void ch_viewdistance(const char* args, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.viewdistance = atof(args) * 100;
}

void ch_fadestart(const char* args, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.fadestart = atof(args);
}

void ch_slomo(const char* args, GameState& gamestate, GameAssets&, Console&)
{
	gamestate.slomospeed = atof(args);
	gamestate.slomo = !gamestate.slomo;
	gamestate.slomodelay = 1000;
}

void ch_slofreq(const char* args, GameState&, GameAssets&, Console&)
{
	slomofreq = atof(args);
}

void ch_skytint(const char* args, GameState& gamestate, GameAssets& assets, Console&)
{
	sscanf(args, "%f%f%f", &gamestate.skyboxr, &gamestate.skyboxg, &gamestate.skyboxb);

	gamestate.skyboxlightr = gamestate.skyboxr;
	gamestate.skyboxlightg = gamestate.skyboxg;
	gamestate.skyboxlightb = gamestate.skyboxb;

	SetUpLighting(gamestate);

	assets.terrain->DoShadows(Tutorial::active, gamestate.texscale, gamestate.light, gamestate.skyboxtexture, [&]() {Game::LoadingScreen(gamestate, assets); });
	Object::DoShadows(gamestate.skyboxtexture, gamestate.light, *assets.terrain);
}

void ch_skylight(const char* args, GameState& gamestate, GameAssets& assets, Console&)
{
	sscanf(args, "%f%f%f", &gamestate.skyboxlightr, &gamestate.skyboxlightg, &gamestate.skyboxlightb);

	SetUpLighting(gamestate);

	assets.terrain->DoShadows(Tutorial::active, gamestate.texscale, gamestate.light, gamestate.skyboxtexture, [&]() {Game::LoadingScreen(gamestate, assets); });
	Object::DoShadows(gamestate.skyboxtexture, gamestate.light, *assets.terrain);
}

void ch_skybox(const char*, GameState& gamestate, GameAssets& assets, Console&)
{
	gamestate.skyboxtexture = !gamestate.skyboxtexture;

	SetUpLighting(gamestate);

	assets.terrain->DoShadows(Tutorial::active, gamestate.texscale, gamestate.light, gamestate.skyboxtexture, [&]() {Game::LoadingScreen(gamestate, assets); });
	Object::DoShadows(gamestate.skyboxtexture, gamestate.light, *assets.terrain);
}
