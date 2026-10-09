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

#include "Menu/Menu.hpp"

#include "Game.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "KeyCapture.hpp"

#include "Audio/openal_wrapper.hpp"
#include "Graphic/gamegl.hpp"
#include "Level/Campaign.hpp"
#include "User/Settings.hpp"
#include "Utils/Input.hpp"
#include "Version.hpp"
#include "WindowContext.hpp"

// Should not be needed, Menu should call methods from other classes to launch maps and challenges and so on
#include "Level/Awards.hpp"

#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace Game;

extern std::vector<CampaignLevel> campaignlevels;
extern float musicvolume[4];
extern float oldmusicvolume[4];
extern int leveltheme;

int entername = 0;
std::string newusername = "";
unsigned newuserselected = 0;
float newuserblinkdelay = 0;
bool newuserblink = false;

std::vector<MenuItem> Menu::items;

MenuItem::MenuItem(MenuItemType _type, int _id, const std::string& _text, Texture _texture,
                   int _x, int _y, int _w, int _h, float _r, float _g, float _b,
                   float _linestartsize, float _lineendsize)
    : type(_type)
    , id(_id)
    , text(_text)
    , texture(_texture)
    , x(_x)
    , y(_y)
    , w(_w)
    , h(_h)
    , r(_r)
    , g(_g)
    , b(_b)
    , effectfade(0)
    , linestartsize(_linestartsize)
    , lineendsize(_lineendsize)
{
    if (type == MenuItem::BUTTON) {
        if (w == -1) {
            w = text.length() * 10;
        }
        if (h == -1) {
            h = 20;
        }
    }
}

void Menu::clearMenu()
{
    items.clear();
}

void Menu::addLabel(int id, const std::string& labeltext, int x, int y, float r, float g, float b)
{
    items.emplace_back(MenuItem::LABEL, id, labeltext, Texture(), x, y, -1, -1, r, g, b);
}
void Menu::addButton(int id, const std::string& buttontext, int x, int y, float r, float g, float b)
{
    items.emplace_back(MenuItem::BUTTON, id, buttontext, Texture(), x, y, -1, -1, r, g, b);
}
void Menu::addImage(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b)
{
    items.emplace_back(MenuItem::IMAGE, id, "", texture, x, y, w, h, r, g, b);
}
void Menu::addButtonImage(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b)
{
    items.emplace_back(MenuItem::IMAGEBUTTON, id, "", texture, x, y, w, h, r, g, b);
}
void Menu::addMapLine(int x, int y, int w, int h, float startsize, float endsize, float r, float g, float b)
{
    items.emplace_back(MenuItem::MAPLINE, -1, "", Texture(), x, y, w, h, r, g, b, startsize, endsize);
}
void Menu::addMapMarker(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b)
{
    items.emplace_back(MenuItem::MAPMARKER, id, "", texture, x, y, w, h, r, g, b);
}
void Menu::addMapLabel(int id, const std::string& labeltext, int x, int y, float r, float g, float b)
{
    items.emplace_back(MenuItem::MAPLABEL, id, labeltext, Texture(), x, y, -1, -1, r, g, b);
}

void Menu::setText(int id, const std::string& newtext)
{
    for (std::vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        if (it->id == id) {
            it->text = newtext;
            it->w = it->text.length() * 10;
            break;
        }
    }
}

void Menu::setText(int id, const std::string& newtext, int x, int y, int w, int h)
{
    for (std::vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        if (it->id == id) {
            it->text = newtext;
            it->x = x;
            it->y = y;
            if (w == -1) {
                it->w = it->text.length() * 10;
            }
            if (h == -1) {
                it->h = 20;
            }
            break;
        }
    }
}

int Menu::getSelected(int mousex, int mousey)
{
    for (std::vector<MenuItem>::reverse_iterator it = items.rbegin(); it != items.rend(); it++) {
        if (it->type == MenuItem::BUTTON || it->type == MenuItem::IMAGEBUTTON || it->type == MenuItem::MAPMARKER) {
            int mx = mousex;
            int my = mousey;
            if (it->type == MenuItem::MAPMARKER) {
                mx -= 1;
                my += 2;
            }
            if (mx >= it->x && mx < it->x + it->w && my >= it->y && my < it->y + it->h) {
                return it->id;
            }
        }
    }
    return -1;
}

void Menu::handleFadeEffect(GameState& gamestate)
{
    for (std::vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        if (it->id == gamestate.selected) {
            it->effectfade += gamestate.multiplier * 5;
            if (it->effectfade > 1) {
                it->effectfade = 1;
            }
        } else {
            it->effectfade -= gamestate.multiplier * 5;
            if (it->effectfade < 0) {
                it->effectfade = 0;
            }
        }
    }
}

void Menu::drawItems(GameState& gamestate, GameAssets& assets)
{
    handleFadeEffect(gamestate);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    for (std::vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        switch (it->type) {
            case MenuItem::IMAGE:
            case MenuItem::IMAGEBUTTON:
            case MenuItem::MAPMARKER:
                glColor4f(it->r, it->g, it->b, 1);
                glPushMatrix();
                if (it->type == MenuItem::MAPMARKER) {
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glTranslatef(2.5, -4.5, 0); //from old code
                } else {
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                }
                it->texture.bind();
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(it->x, it->y, 0);
                glTexCoord2f(1, 0);
                glVertex3f(it->x + it->w, it->y, 0);
                glTexCoord2f(1, 1);
                glVertex3f(it->x + it->w, it->y + it->h, 0);
                glTexCoord2f(0, 1);
                glVertex3f(it->x, it->y + it->h, 0);
                glEnd();
                if (it->type != MenuItem::IMAGE) {
                    //mouseover highlight
                    for (int i = 0; i < 10; i++) {
                        if (1 - ((float)i) / 10 - (1 - it->effectfade) > 0) {
                            glColor4f(it->r, it->g, it->b, (1 - ((float)i) / 10 - (1 - it->effectfade)) * .25);
                            glBegin(GL_QUADS);
                            glTexCoord2f(0, 0);
                            glVertex3f(it->x - ((float)i) * 1 / 2, it->y - ((float)i) * 1 / 2, 0);
                            glTexCoord2f(1, 0);
                            glVertex3f(it->x + it->w + ((float)i) * 1 / 2, it->y - ((float)i) * 1 / 2, 0);
                            glTexCoord2f(1, 1);
                            glVertex3f(it->x + it->w + ((float)i) * 1 / 2, it->y + it->h + ((float)i) * 1 / 2, 0);
                            glTexCoord2f(0, 1);
                            glVertex3f(it->x - ((float)i) * 1 / 2, it->y + it->h + ((float)i) * 1 / 2, 0);
                            glEnd();
                        }
                    }
                }
                glPopMatrix();
                break;
            case MenuItem::LABEL:
            case MenuItem::BUTTON:
                glColor4f(it->r, it->g, it->b, 1);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                assets.text.glPrint(it->x, it->y, it->text.c_str(), 0, 1, 640, 480);
                if (it->type != MenuItem::LABEL) {
                    //mouseover highlight
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    for (int i = 0; i < 15; i++) {
                        if (1 - ((float)i) / 15 - (1 - it->effectfade) > 0) {
                            glColor4f(it->r, it->g, it->b, (1 - ((float)i) / 10 - (1 - it->effectfade)) * .25);
                            assets.text.glPrint(it->x - ((float)i), it->y, it->text.c_str(), 0, 1 + ((float)i) / 70, 640, 480);
                        }
                    }
                }
                break;
            case MenuItem::MAPLABEL:
                assets.text.glPrintOutlined(0.9, 0, 0, 1, it->x, it->y, it->text.c_str(), 0, 0.6, 640, 480);
                break;
            case MenuItem::MAPLINE: {
                Vector3 linestart;
                linestart.x = it->x;
                linestart.y = it->y;
                linestart.z = 0;
                Vector3 lineend;
                lineend.x = it->x + it->w;
                lineend.y = it->y + it->h;
                lineend.z = 0;
                Vector3 offset = lineend - linestart;
                Vector3 fac = offset;
                Normalise(&fac);
                offset = DoRotation(offset, 0, 0, 90);
                Normalise(&offset);

                linestart += fac * 4 * it->linestartsize;
                lineend -= fac * 4 * it->lineendsize;

                glDisable(GL_TEXTURE_2D);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(it->r, it->g, it->b, 1);
                glPushMatrix();
                glTranslatef(2, -5, 0); //from old code
                glBegin(GL_QUADS);
                glVertex3f(linestart.x - offset.x * it->linestartsize, linestart.y - offset.y * it->linestartsize, 0.0f);
                glVertex3f(linestart.x + offset.x * it->linestartsize, linestart.y + offset.y * it->linestartsize, 0.0f);
                glVertex3f(lineend.x + offset.x * it->lineendsize, lineend.y + offset.y * it->lineendsize, 0.0f);
                glVertex3f(lineend.x - offset.x * it->lineendsize, lineend.y - offset.y * it->lineendsize, 0.0f);
                glEnd();
                glPopMatrix();
                glEnable(GL_TEXTURE_2D);
            } break;
            default:
            case MenuItem::NONE:
                break;
        }
    }
}

void Menu::updateSettingsMenu(GameState& gamestate)
{
    std::string sbuf = std::string("Resolution: ") + std::to_string(gamestate.newscreenwidth) + "*" + std::to_string(gamestate.newscreenheight);
    if (((float)gamestate.newscreenwidth <= (float)gamestate.newscreenheight * 1.61) && ((float)gamestate.newscreenwidth >= (float)gamestate.newscreenheight * 1.59)) {
        sbuf += " (widescreen)";
    }
    setText(0, sbuf);
    setText(14, gamestate.fullscreen ? "Fullscreen: On" : "Fullscreen: Off");
    if (gamestate.newdetail == 0) {
        setText(1, "Detail: Low");
    }
    if (gamestate.newdetail == 1) {
        setText(1, "Detail: Medium");
    }
    if (gamestate.newdetail == 2) {
        setText(1, "Detail: High");
    }
    if (gamestate.bloodtoggle == 0) {
        setText(2, "Blood: Off");
    }
    if (gamestate.bloodtoggle == 1) {
        setText(2, "Blood: On, low gamestate.detail");
    }
    if (gamestate.bloodtoggle == 2) {
        setText(2, "Blood: On, high gamestate.detail (slower)");
    }
    setText(4, gamestate.ismotionblur ? "Blur Effects: Enabled (less compatible)" : "Blur Effects: Disabled (more compatible)");
    setText(5, gamestate.decalstoggle ? "Decals: Enabled (slower)" : "Decals: Disabled");
    setText(6, gamestate.musictoggle ? "Music: Enabled" : "Music: Disabled");
    setText(9, gamestate.invertmouse ? "Invert mouse: Yes" : "Invert mouse: No");
    setText(10, std::string("Mouse Speed: ") + std::to_string(int(gamestate.usermousesensitivity * 5)));
    setText(11, std::string("Volume: ") + std::to_string(int(gamestate.volume * 100)) + "%");
    setText(13, gamestate.showdamagebar ? "Damage Bar: On" : "Damage Bar: Off");
    if ((gamestate.newdetail == gamestate.detail) && (gamestate.newscreenheight == (int)gamestate.screenheight) && (gamestate.newscreenwidth == (int)gamestate.screenwidth)) {
        setText(8, "Back");
    } else {
        setText(8, "Back (some changes take effect next time Lugaru is opened)");
    }
}

void Menu::updateStereoConfigMenu(GameState& gamestate)
{
    setText(0, std::string("Stereo mode: ") + StereoModeName(gamestate.newstereomode));
    setText(1, std::string("Stereo separation: ") + std::to_string(gamestate.stereoseparation));
    setText(2, std::string("Reverse stereo: ") + (gamestate.stereoreverse ? "Yes" : "No"));
}

void Menu::updateControlsMenu(GameState& gamestate, KeyCapture& keycapture)
{
    setText(0, (std::string) "Forwards: " + (keycapture.keyselect == 0 ? "_" : Input::keyToChar(gamestate.forwardkey)));
    setText(1, (std::string) "Back: " + (keycapture.keyselect == 1 ? "_" : Input::keyToChar(gamestate.backkey)));
    setText(2, (std::string) "Left: " + (keycapture.keyselect == 2 ? "_" : Input::keyToChar(gamestate.leftkey)));
    setText(3, (std::string) "Right: " + (keycapture.keyselect == 3 ? "_" : Input::keyToChar(gamestate.rightkey)));
    setText(4, (std::string) "Crouch: " + (keycapture.keyselect == 4 ? "_" : Input::keyToChar(gamestate.crouchkey)));
    setText(5, (std::string) "Jump: " + (keycapture.keyselect == 5 ? "_" : Input::keyToChar(gamestate.jumpkey)));
    setText(6, (std::string) "Draw: " + (keycapture.keyselect == 6 ? "_" : Input::keyToChar(gamestate.drawkey)));
    setText(7, (std::string) "Throw: " + (keycapture.keyselect == 7 ? "_" : Input::keyToChar(gamestate.throwkey)));
    setText(8, (std::string) "Attack: " + (keycapture.keyselect == 8 ? "_" : Input::keyToChar(gamestate.attackkey)));
    if (gamestate.devtools) {
        setText(9, (std::string) "Console: " + (keycapture.keyselect == 9 ? "_" : Input::keyToChar(gamestate.consolekey)));
    }
}

/*
Values of gamestate.mainmenu :
1 Main menu
2 Menu pause (resume/end game)
3 Option menu
4 Controls configuration menu
5 Main game menu (choose level or challenge)
6 Deleting user menu
7 User managment menu (select/add)
8 Choose gamestate.difficulty menu
9 Challenge level selection menu
10 End of the campaign congratulation (is that really a menu?)
11 Same that 9 ??? => unused
18 stereo configuration
*/

void Menu::Load(GameState& gamestate, GameAssets& assets, KeyCapture& keycapture)
{
    clearMenu();
    switch (gamestate.mainmenu) {
        case 1:
        case 2:
            addImage(0, assets.Mainmenuitems[0], 150, 480 - 128, 256, 128);
            addButtonImage(1, assets.Mainmenuitems[gamestate.mainmenu == 1 ? 1 : 5], 18, 480 - 152 - 32, 128, 32);
            addButtonImage(2, assets.Mainmenuitems[2], 18, 480 - 228 - 32, 112, 32);
            addButtonImage(3, assets.Mainmenuitems[gamestate.mainmenu == 1 ? 3 : 6], 18, 480 - 306 - 32, gamestate.mainmenu == 1 ? 68 : 132, 32);
            addLabel(-1, VERSION_NUMBER + VERSION_SUFFIX, 640 - 100, 10);
            break;
        case 3:
            addButton(0, "", 10 + 20, 440);
            addButton(14, "", 10 + 400, 440);
            addButton(1, "", 10 + 60, 405);
            addButton(2, "", 10 + 70, 370);
            addButton(4, "", 10, 335);
            addButton(5, "", 10 + 60, 300);
            addButton(6, "", 10 + 70, 265);
            addButton(9, "", 10, 230);
            addButton(10, "", 20, 195);
            addButton(11, "", 10 + 60, 160);
            addButton(13, "", 30, 125);
            addButton(7, "-Configure Controls-", 10 + 15, 90);
            addButton(12, "-Configure Stereo -", 10 + 15, 55);
            addButton(8, "Back", 10, 10);
            updateSettingsMenu(gamestate);
            break;
        case 4:
            addButton(0, "", 10, 400);
            addButton(1, "", 10 + 40, 360);
            addButton(2, "", 10 + 40, 320);
            addButton(3, "", 10 + 30, 280);
            addButton(4, "", 10 + 20, 240);
            addButton(5, "", 10 + 40, 200);
            addButton(6, "", 10 + 40, 160);
            addButton(7, "", 10 + 30, 120);
            addButton(8, "", 10 + 20, 80);
            if (gamestate.devtools) {
                addButton(9, "", 10 + 10, 40);
            }
            addButton(gamestate.devtools ? 10 : 9, "Back", 10, 10);
            updateControlsMenu(gamestate, keycapture);
            break;
        case 5: {
            LoadCampaign(gamestate, assets);
            addLabel(-1, Account::active().getName(), 5, 400);
            addButton(1, "Tutorial", 5, 300);
            addButton(2, "Challenge", 5, 240);
            addButton(3, "Delete User", 400, 10);
            addButton(4, "Main Menu", 5, 10);
            addButton(5, "Change User", 5, 180);
            addButton(6, "Campaign : " + Account::active().getCurrentCampaign(), 200, 420);

            //show campaign map
            //with (2,-5) offset from old code
            addImage(-1, assets.Mainmenuitems[7], 150 + 2, 60 - 5, 400, 400);
            //show levels
            int numlevels = Account::active().getCampaignChoicesMade();
            numlevels += numlevels > 0 ? campaignlevels[numlevels - 1].nextlevel.size() : 1;
            for (int i = 0; i < numlevels; i++) {
                Vector3 midpoint = campaignlevels[i].getCenter();
                float itemsize = campaignlevels[i].getWidth();
                const bool active = (i >= Account::active().getCampaignChoicesMade());
                if (!active) {
                    itemsize /= 2;
                }

                if (i >= 1) {
                    Vector3 start = campaignlevels[i - 1].getCenter();
                    addMapLine(start.x, start.y, midpoint.x - start.x, midpoint.y - start.y, 0.5, active ? 1 : 0.5, active ? 1 : 0.5, 0, 0);
                }
                addMapMarker(NB_CAMPAIGN_MENU_ITEM + i, assets.Mapcircletexture,
                             midpoint.x - itemsize / 2, midpoint.y - itemsize / 2, itemsize, itemsize, active ? 1 : 0.5, 0, 0);

                if (active) {
                    addMapLabel(-2, campaignlevels[i].description,
                                campaignlevels[i].getStartX() + 10,
                                campaignlevels[i].getStartY() - 4);
                }
            }
        } break;
        case 6:
            addLabel(-1, "Are you sure you want to delete this user?", 10, 400);
            addButton(1, "Yes", 10, 360);
            addButton(2, "No", 10, 320);
            break;
        case 7:
            if (Account::getNbAccounts() < 8) {
                addButton(0, "New User", 10, 400);
            } else {
                addLabel(0, "No More Users", 10, 400);
            }
            addLabel(-2, "", 20, 400);
            addButton(Account::getNbAccounts() + 1, "Back", 10, 10);
            for (int i = 0; i < Account::getNbAccounts(); i++) {
                addButton(i + 1, Account::get(i).getName(), 10, 340 - 20 * (i + 1));
            }
            break;
        case 8:
            addButton(0, "Easier", 10, 400);
            addButton(1, "Difficult", 10, 360);
            addButton(2, "Insane", 10, 320);
            break;
        case 9:
            for (int i = 0; i < gamestate.numchallengelevels; i++) {
               std::string name = "Level ";
                name += std::to_string(i + 1);
                if (name.size() < 17) {
                    name.append((17 - name.size()), ' ');
                }
                name += std::to_string(int(Account::active().getHighScore(i)));
                if (name.size() < 32) {
                    name.append((32 - name.size()), ' ');
                }
                int fasttime = (int)round(Account::active().getFastTime(i));
                name += std::to_string(int((fasttime - fasttime % 60) / 60));
                name += ":";
                if (fasttime % 60 < 10) {
                    name += "0";
                }
                name += std::to_string(fasttime % 60);

                addButton(i, name, 10, 400 - i * 25, i > Account::active().getProgress() ? 0.5 : 1, 0, 0);
            }

            addButton(-1, "             High Score      Best Time", 10, 440);
            addButton(gamestate.numchallengelevels, "Back", 10, 10);
            break;
        case 10: {
            addLabel(0, campaignEndText[0], 80, 330);
            addLabel(1, campaignEndText[1], 80, 300);
            addLabel(2, campaignEndText[2], 80, 270);
            addButton(3, "Back", 10, 10);
            addLabel(4, std::string("Your score:         ") + std::to_string((int)Account::active().getCampaignScore()), 190, 200);
            addLabel(5, std::string("Highest score:      ") + std::to_string((int)Account::active().getCampaignHighScore()), 190, 180);
        } break;
        case 18:
            addButton(0, "", 70, 400);
            addButton(1, "", 10, 360);
            addButton(2, "", 40, 320);
            addButton(3, "Back", 10, 10);
            updateStereoConfigMenu(gamestate);
            break;
    }
}

void Menu::startChallengeLevel(int challengelevel, GameState& gamestate, GameAssets& assets, Console& console)
{
    fireSound();
    flash(gamestate);

    startbonustotal = 0;

    gamestate.loading = 2;
    gamestate.loadtime = 0;
    gamestate.targetlevel = challengelevel;
    if (gamestate.firstLoadDone) {
        TickOnceAfter(gamestate, assets, console);
    } else {
        LoadStuff(gamestate, assets);
    }
    LoadLevel(challengelevel, gamestate, assets, console);
    gamestate.campaign = 0;

    gamestate.mainmenu = 0;
    gamestate.gameon = 1;
    pause_sound(stream_menutheme);
}

namespace
{
// Defined near the capture thread below, with the rest of the file-local
// machinery for the key capture. Declared here because Menu::Tick, which is the
// only caller, comes first.
void applyCapturedKey(GameState& gamestate, KeyCapture& keycapture);
}

void Menu::Tick(GameState& gamestate, GameAssets& assets, KeyCapture& keycapture, Console& console)
{
    // Answer the capture thread's request for the menu to be rebuilt. It used to
    // rebuild it itself, in the middle of a frame the main thread was running,
    // which put two threads on Menu::items at once.
    //
    // First thing in the body, and not inside any branch below, because this has
    // to happen on the frames the capture thread is waiting as well as on the one
    // it finishes on: everything after the controls-menu input handling below only
    // runs when `waiting` is already clear. takeReloadRequest() also acquires, so
    // the scancode the thread parked is visible to what follows.
    if (keycapture.takeReloadRequest()) {
        applyCapturedKey(gamestate, keycapture);
        Load(gamestate, assets, keycapture);
    }

    //escape key pressed
    if (Input::isKeyPressed(SDL_SCANCODE_ESCAPE) &&
        (gamestate.mainmenu >= 3) && (gamestate.mainmenu != 8) && !((gamestate.mainmenu == 7) && entername)) {
        gamestate.selected = -1;
        //finished with settings menu
        if (gamestate.mainmenu == 3) {
            SaveSettings(gamestate);
        }
        //effects
        if (gamestate.mainmenu >= 3 && gamestate.mainmenu != 8) {
            fireSound();
            flash(gamestate);
        }
        //go back
        switch (gamestate.mainmenu) {
            case 3:
            case 5:
                gamestate.mainmenu = gamestate.gameon ? 2 : 1;
                break;
            case 4:
            case 18:
                gamestate.mainmenu = 3;
                break;
            case 6:
            case 7:
            case 9:
            case 10:
                gamestate.mainmenu = 5;
                break;
        }
    }

    //menu buttons
    gamestate.selected = getSelected(gamestate.mousecoordh * 640 / gamestate.screenwidth, 480 - gamestate.mousecoordv * 480 / gamestate.screenheight);

    // some specific case where we do something even if the left mouse button is not pressed.
    if ((gamestate.mainmenu == 5) && (gamestate.endgame == 2)) {
        Account::active().endGame();
        gamestate.endgame = 0;
    }
    if (gamestate.mainmenu == 10) {
        gamestate.endgame = 2;
    }
    if (gamestate.mainmenu == 18 && Input::isKeyPressed(MOUSEBUTTON_RIGHT) && gamestate.selected == 1) {
        gamestate.stereoseparation -= 0.001;
        updateStereoConfigMenu(gamestate);
    }

    static int oldmainmenu = gamestate.mainmenu;

    if (Input::MouseClicked() && (gamestate.selected >= 0)) { // handling of the left mouse clic in menus
        std::set<std::pair<int, int>>::iterator newscreenresolution;
        switch (gamestate.mainmenu) {
            case 1:
            case 2:
                switch (gamestate.selected) {
                    case 1:
                        if (gamestate.gameon) { //resume
                            gamestate.mainmenu = 0;
                            pause_sound(stream_menutheme);
                            resume_stream(leveltheme);
                        } else { //new game
                            fireSound(firestartsound);
                            flash(gamestate);
                            gamestate.mainmenu = (Account::hasActive() ? 5 : 7);
                            gamestate.selected = -1;
                        }
                        break;
                    case 2: //options
                        fireSound();
                        flash(gamestate);
                        gamestate.mainmenu = 3;
                        if (gamestate.newdetail > 2) {
                            gamestate.newdetail = gamestate.detail;
                        }
                        if (gamestate.newdetail < 0) {
                            gamestate.newdetail = gamestate.detail;
                        }
                        if (gamestate.newscreenwidth > 3000) {
                            gamestate.newscreenwidth = gamestate.screenwidth;
                        }
                        if (gamestate.newscreenwidth < 0) {
                            gamestate.newscreenwidth = gamestate.screenwidth;
                        }
                        if (gamestate.newscreenheight > 3000) {
                            gamestate.newscreenheight = gamestate.screenheight;
                        }
                        if (gamestate.newscreenheight < 0) {
                            gamestate.newscreenheight = gamestate.screenheight;
                        }
                        break;
                    case 3:
                        fireSound();
                        flash(gamestate);
                        if (gamestate.gameon) { //end game
                            gamestate.gameon = 0;
                            gamestate.mainmenu = 1;
                        } else { //quit
                            gamestate.tryquit = 1;
                            pause_sound(stream_menutheme);
                        }
                        break;
                }
                break;
            case 3:
                fireSound();
                switch (gamestate.selected) {
                    case 0:
                        newscreenresolution = WindowContext::resolutions.find(std::make_pair(gamestate.newscreenwidth, gamestate.newscreenheight));
                        /* Next one (end() + 1 is also end() so the ++ is safe even if it was not found) */
                        newscreenresolution++;
                        if (newscreenresolution == WindowContext::resolutions.end()) {
                            /* It was the last one (or not found), go back to the beginning */
                            newscreenresolution = WindowContext::resolutions.begin();
                        }
                        gamestate.newscreenwidth = newscreenresolution->first;
                        gamestate.newscreenheight = newscreenresolution->second;
                        break;
                    case 1:
                        gamestate.newdetail++;
                        if (gamestate.newdetail > 2) {
                            gamestate.newdetail = 0;
                        }
                        break;
                    case 2:
                        gamestate.bloodtoggle++;
                        if (gamestate.bloodtoggle > 2) {
                            gamestate.bloodtoggle = 0;
                        }
                        break;
                    case 4:
                        gamestate.ismotionblur = !gamestate.ismotionblur;
                        break;
                    case 5:
                        gamestate.decalstoggle = !gamestate.decalstoggle;
                        break;
                    case 6:
                        gamestate.musictoggle = !gamestate.musictoggle;
                        if (gamestate.musictoggle) {
                            emit_stream_np(stream_menutheme);
                        } else {
                            pause_sound(leveltheme);
                            pause_sound(stream_fighttheme);
                            pause_sound(stream_menutheme);

                            for (int i = 0; i < 4; i++) {
                                oldmusicvolume[i] = 0;
                                musicvolume[i] = 0;
                            }
                        }
                        break;
                    case 7: // controls
                        flash(gamestate);
                        gamestate.mainmenu = 4;
                        gamestate.selected = -1;
                        keycapture.keyselect = -1;
                        break;
                    case 8:
                        flash(gamestate);
                        SaveSettings(gamestate);
                        gamestate.mainmenu = gamestate.gameon ? 2 : 1;
                        break;
                    case 9:
                        gamestate.invertmouse = !gamestate.invertmouse;
                        break;
                    case 10:
                        gamestate.usermousesensitivity += .2;
                        if (gamestate.usermousesensitivity > 2) {
                            gamestate.usermousesensitivity = .2;
                        }
                        break;
                    case 11:
                        gamestate.volume += .1f;
                        if (gamestate.volume > 1.0001f) {
                            gamestate.volume = 0;
                        }
                        OPENAL_SetSFXMasterVolume((int)(gamestate.volume * 255));
                        break;
                    case 12:
                        flash(gamestate);
                        gamestate.newstereomode = gamestate.stereomode;
                        gamestate.mainmenu = 18;
                        keycapture.keyselect = -1;
                        break;
                    case 13:
                        gamestate.showdamagebar = !gamestate.showdamagebar;
                        break;
                    case 14:
                        WindowContext::toggleFullscreen(gamestate);
                        break;
                }
                updateSettingsMenu(gamestate);
                break;
            case 4:
                if (!keycapture.waiting) {
                    fireSound();
                    if (gamestate.selected < (gamestate.devtools ? 10 : 9) && keycapture.keyselect == -1) {
                        keycapture.keyselect = gamestate.selected;
                    }
                    if (keycapture.keyselect != -1) {
                        setKeySelected(keycapture);
                    }
                    if (gamestate.selected == (gamestate.devtools ? 10 : 9)) {
                        flash(gamestate);
                        gamestate.mainmenu = 3;
                    }
                }
                updateControlsMenu(gamestate, keycapture);
                break;
            case 5:
                fireSound();
                flash(gamestate);
                if ((gamestate.selected - NB_CAMPAIGN_MENU_ITEM >= Account::active().getCampaignChoicesMade())) {
                    startbonustotal = 0;

                    gamestate.loading = 2;
                    gamestate.loadtime = 0;
                    gamestate.targetlevel = 7;
                    if (gamestate.firstLoadDone) {
                        TickOnceAfter(gamestate, assets, console);
                    } else {
                        LoadStuff(gamestate, assets);
                    }
                    gamestate.whichchoice = gamestate.selected - NB_CAMPAIGN_MENU_ITEM - Account::active().getCampaignChoicesMade();
                    gamestate.actuallevel = (Account::active().getCampaignChoicesMade() > 0 ? campaignlevels[Account::active().getCampaignChoicesMade() - 1].nextlevel[gamestate.whichchoice] : 0);
                    gamestate.visibleloading = true;
                    gamestate.stillloading = 1;
                    LoadLevel(campaignlevels[gamestate.actuallevel].mapname.c_str(), false, gamestate, assets, console);
                    gamestate.campaign = 1;
                    gamestate.mainmenu = 0;
                    gamestate.gameon = 1;
                    pause_sound(stream_menutheme);
                }
                switch (gamestate.selected) {
                    case 1:
                        startbonustotal = 0;

                        gamestate.loading = 2;
                        gamestate.loadtime = 0;
                        gamestate.targetlevel = -1;
                        if (gamestate.firstLoadDone) {
                            TickOnceAfter(gamestate, assets, console);
                        } else {
                            LoadStuff(gamestate, assets);
                        }
                        LoadLevel(-1, gamestate, assets, console);

                        gamestate.mainmenu = 0;
                        gamestate.gameon = 1;
                        pause_sound(stream_menutheme);
                        break;
                    case 2:
                        gamestate.mainmenu = 9;
                        break;
                    case 3:
                        gamestate.mainmenu = 6;
                        break;
                    case 4:
                        gamestate.mainmenu = (gamestate.gameon ? 2 : 1);
                        break;
                    case 5:
                        gamestate.mainmenu = 7;
                        break;
                    case 6:
                        std::vector<std::string> campaigns = ListCampaigns();
                        std::vector<std::string>::iterator c;
                        if ((c = find(campaigns.begin(), campaigns.end(), Account::active().getCurrentCampaign())) == campaigns.end()) {
                            if (!campaigns.empty()) {
                                Account::active().setCurrentCampaign(campaigns.front());
                            }
                        } else {
                            c++;
                            if (c == campaigns.end()) {
                                c = campaigns.begin();
                            }
                            Account::active().setCurrentCampaign(*c);
                        }
                        Load(gamestate, assets, keycapture);
                        break;
                }
                break;
            case 6:
                fireSound();
                if (gamestate.selected == 1) {
                    flash(gamestate);
                    Account::destroyActive();
                    gamestate.mainmenu = 7;
                } else if (gamestate.selected == 2) {
                    flash(gamestate);
                    gamestate.mainmenu = 5;
                }
                break;
            case 7:
                fireSound();
                if (gamestate.selected == 0 && Account::getNbAccounts() < 8) {
                    entername = 1;
                } else if (gamestate.selected < Account::getNbAccounts() + 1) {
                    flash(gamestate);
                    gamestate.mainmenu = 5;
                    Account::setActive(gamestate.selected - 1);
                } else if (gamestate.selected == Account::getNbAccounts() + 1) {
                    flash(gamestate);
                    if (Account::hasActive()) {
                        gamestate.mainmenu = 5;
                    } else {
                        gamestate.mainmenu = 1;
                    }
                    newusername.clear();
                    newuserselected = 0;
                    entername = 0;
                }
                break;
            case 8:
                fireSound();
                flash(gamestate);
                if (gamestate.selected <= 2) {
                    Account::active().setDifficulty(gamestate.selected);
                }
                gamestate.mainmenu = 5;
                break;
            case 9:
                if (gamestate.selected < gamestate.numchallengelevels && gamestate.selected <= Account::active().getProgress()) {
                    startChallengeLevel(gamestate.selected, gamestate, assets, console);
                }
                if (gamestate.selected == gamestate.numchallengelevels) {
                    fireSound();
                    flash(gamestate);
                    gamestate.mainmenu = 5;
                }
                break;
            case 10:
                if (gamestate.selected == 3) {
                    fireSound();
                    flash(gamestate);
                    gamestate.mainmenu = 5;
                }
                break;
            case 18:
                if (gamestate.selected == 1) {
                    gamestate.stereoseparation += 0.001;
                } else {
                    fireSound();
                    if (gamestate.selected == 0) {
                        gamestate.newstereomode = (StereoMode)(gamestate.newstereomode + 1);
                        while (!CanInitStereo(gamestate.newstereomode)) {
                            printf("Failed to initialize mode %s (%i)\n", StereoModeName(gamestate.newstereomode).c_str(), gamestate.newstereomode);
                            gamestate.newstereomode = (StereoMode)(gamestate.newstereomode + 1);
                            if (gamestate.newstereomode >= stereoCount) {
                                gamestate.newstereomode = stereoNone;
                            }
                        }
                    } else if (gamestate.selected == 2) {
                        gamestate.stereoreverse = !gamestate.stereoreverse;
                    } else if (gamestate.selected == 3) {
                        flash(gamestate);
                        gamestate.mainmenu = 3;

                        gamestate.stereomode = gamestate.newstereomode;
                        InitStereo(gamestate.stereomode, WindowContext::kContextWidth, WindowContext::kContextHeight);
                    }
                }
                updateStereoConfigMenu(gamestate);
                break;
        }
    }

    OPENAL_SetFrequency(channels[stream_menutheme]);

    if (entername) {
        inputText(newusername, &newuserselected, gamestate, keycapture);
        if (!keycapture.waiting) {                 // the input as finished
            if (!newusername.empty()) { // with enter
                Account::add(std::string(newusername));

                gamestate.mainmenu = 8;

                flash(gamestate);

                fireSound(firestartsound);

                newusername.clear();

                newuserselected = 0;
            }
            entername = 0;
            Load(gamestate, assets, keycapture);
        }

        newuserblinkdelay -= gamestate.multiplier;
        if (newuserblinkdelay <= 0) {
            newuserblinkdelay = .3;
            newuserblink = !newuserblink;
        }
    }

    if (entername) {
        setText(0, newusername, 20, 400, -1, -1);
        setText(-2, newuserblink ? "_" : "", 20 + newuserselected * 10, 400, -1, -1);
    }

    if (oldmainmenu != gamestate.mainmenu) {
        Load(gamestate, assets, keycapture);
    }
    oldmainmenu = gamestate.mainmenu;
}

// SDL_CreateThread carries one pointer and the thread needs two: the caller's
// GameState, which it writes the captured key into, and the KeyCapture beside it,
// which is the handshake the two threads agree on. It gets this record on the
// heap and deletes it on the way out. Both pointers stay valid because
// joinKeySelectThread() runs before either object leaves scope. Anonymous, so
// neither name can collide with another translation unit's: an ODR violation
// would be diagnosed as one symbol quietly replacing another rather than as a
// redeclaration error.
//
// The thread has no GameAssets. It used to, for the Menu::Load it ended with, and
// that was the point at which a thread stopped being a reader of shared state
// and became a writer of it: Menu::Load clears and rebuilds the file-static
// Menu::items the main thread walks in handleFadeEffect, and reads
// assets.Mainmenuitems and assets.Mapcircletexture while doing it. Dropping the
// call dropped the last route from this thread to either.
//
// It carries no GameState either. The thread used to assign the captured
// scancode into one of the ten keybind members, which are plain unsigned short
// that the main thread reads every frame - a data race held open for as long as
// a capture was in flight. The scancode now travels back through KeyCapture and
// the main thread writes it.
namespace
{

struct KeySelectArgs
{
    KeyCapture* keycapture;
};

// The main thread's half of a rebind: take the scancode the capture thread
// parked and write it into the keybind it belongs to. This used to run on the
// thread, which made it a writer of the ten non-atomic keybind members while the
// main thread was reading them. ESC cancels, exactly as it did there.
void applyCapturedKey(GameState& gamestate, KeyCapture& keycapture)
{
    const int scancode = keycapture.capturedScancode.load();
    const int row = keycapture.capturedRow.load();
    if (scancode == -1 || scancode == SDL_SCANCODE_ESCAPE) {
        return;
    }

    Game::fireSound();
    switch (row) {
        case 0:
            gamestate.forwardkey = static_cast<unsigned short>(scancode);
            break;
        case 1:
            gamestate.backkey = static_cast<unsigned short>(scancode);
            break;
        case 2:
            gamestate.leftkey = static_cast<unsigned short>(scancode);
            break;
        case 3:
            gamestate.rightkey = static_cast<unsigned short>(scancode);
            break;
        case 4:
            gamestate.crouchkey = static_cast<unsigned short>(scancode);
            break;
        case 5:
            gamestate.jumpkey = static_cast<unsigned short>(scancode);
            break;
        case 6:
            gamestate.drawkey = static_cast<unsigned short>(scancode);
            break;
        case 7:
            gamestate.throwkey = static_cast<unsigned short>(scancode);
            break;
        case 8:
            gamestate.attackkey = static_cast<unsigned short>(scancode);
            break;
        case 9:
            gamestate.consolekey = static_cast<unsigned short>(scancode);
            break;
        default:
            break;
    }
}

int setKeySelected_thread(void* data)
{
    using namespace Game;
    std::unique_ptr<KeySelectArgs> args(static_cast<KeySelectArgs*>(data));
    KeyCapture& keycapture = *args->keycapture;
    int scancode = -1;
    SDL_Event evenement;
    while (scancode == -1) {
        SDL_WaitEvent(&evenement);
        switch (evenement.type) {
            case SDL_KEYDOWN:
                scancode = evenement.key.keysym.scancode;
                break;
            case SDL_MOUSEBUTTONDOWN:
                scancode = SDL_NUM_SCANCODES + evenement.button.button;
                break;
            default:
                break;
        }
    }
    // Nothing here touches GameState or the audio library. The ten keybind
    // members are plain unsigned short, not atomic, and the main thread reads
    // them every frame to decide what a key press means - so writing them from
    // here was a data race for as long as a capture was in flight. fireSound
    // reaches the audio library's own unsynchronised channel and sample arrays,
    // which is the same problem. Both are the main thread's job now.
    keycapture.capturedScancode = scancode;
    keycapture.capturedRow = keycapture.keyselect.load();
    keycapture.keyselect = -1;
    // Asked for, not done. The keybind is written and the menu that displays it
    // is rebuilt by Menu::Tick on the main thread.
    keycapture.reloadRequested = true;
    // Cleared last, and it is what publishes the stores above: this store
    // releases, and the main thread's next load of `waiting` acquires, so
    // everything written before this line is visible to whatever the main thread
    // does after it sees the flag down.
    keycapture.waiting = false;
    return 0;
}

} // namespace

// The key-capture thread holds a reference to the caller's GameState and to the
// KeyCapture beside it, so its handle is retained and must be joined before
// either goes out of scope. See Menu::joinKeySelectThread().
static SDL_Thread* keyselectthread = nullptr;

void Menu::joinKeySelectThread()
{
	if (keyselectthread != nullptr) {
		SDL_WaitThread(keyselectthread, nullptr);
		keyselectthread = nullptr;
	}
}

void Menu::setKeySelected(KeyCapture& keycapture)
{
    keycapture.waiting = true;
    printf("launch thread\n");
    Menu::joinKeySelectThread();
    KeySelectArgs* args = new KeySelectArgs{ &keycapture };
    keyselectthread = SDL_CreateThread(setKeySelected_thread, NULL, args);
    if (keyselectthread == NULL) {
        delete args;
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
        keycapture.waiting = false;
        return;
    }
}
