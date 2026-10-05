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

#ifndef _MENU_HPP_
#define _MENU_HPP_

#include <string>
#include <vector>

#include "Graphic/Texture.hpp"

struct GameAssets;
struct GameState;

struct MenuItem
{
	enum MenuItemType
	{
		NONE,
		LABEL,
		BUTTON,
		IMAGE,
		IMAGEBUTTON,
		MAPMARKER,
		MAPLINE,
		MAPLABEL
	} type;
	int id;
	std::string text;
	Texture texture;
	int x, y, w, h;
	float r, g, b;
	float effectfade;

	float linestartsize;
	float lineendsize;

	MenuItem(MenuItemType _type, int _id, const std::string& _text, Texture _texture,
		int _x, int _y, int _w, int _h, float _r, float _g, float _b,
		float _linestartsize = 1, float _lineendsize = 1);
};

class Menu
{
public:
	static void clearMenu();
	static void addLabel(int id, const std::string& labeltext, int x, int y, float r = 1, float g = 0, float b = 0);
	static void addButton(int id, const std::string& buttontext, int x, int y, float r = 1, float g = 0, float b = 0);
	static void addImage(int id, Texture texture, int x, int y, int w, int h, float r = 1, float g = 1, float b = 1);
	static void addButtonImage(int id, Texture texture, int x, int y, int w, int h, float r = 1, float g = 1, float b = 1);
	static void addMapLine(int x, int y, int w, int h, float startsize, float endsize, float r, float g, float b);
	static void addMapMarker(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b);
	static void addMapLabel(int id, const std::string& labeltext, int x, int y, float r = 1, float g = 0, float b = 0);
	static void setText(int id, const std::string& newtext);
	static void setText(int id, const std::string& newtext, int x, int y, int w, int h);
	static int getSelected(int mousex, int mousey);
	static void drawItems(GameState& gamestate, GameAssets& assets);

	static void Load(GameState& gamestate);
	static void Tick(GameState& gamestate, GameAssets& assets);
	static void updateSettingsMenu(GameState& gamestate);
	static void updateStereoConfigMenu(GameState& gamestate);
	static void updateControlsMenu(GameState& gamestate);
	static void setKeySelected(GameState& gamestate);
	/**
	 * Waits for the key-capture thread started by setKeySelected() to finish.
	 * Must be called before the GameState that thread references is destroyed.
	 */
	static void joinKeySelectThread();
	static void startChallengeLevel(int challengelevel, GameState& gamestate, GameAssets& assets);

private:
	static void handleFadeEffect(GameState& gamestate);

	static std::vector<MenuItem> items;
};

#endif
