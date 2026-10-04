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

#ifndef _WEAPONS_HPP_
#define _WEAPONS_HPP_

#include "Animation/Skeleton.hpp"
#include "Environment/Terrain.hpp"

#include "Graphic/Models.hpp"
#include "Graphic/Sprite.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"

#include "Math/Vector3.hpp"

#include <cmath>

struct GameState;

#define knife 1
#define sword 2
#define staff 3

class Weapon
{
public:
    Weapon(int type, int owner);

    static void Load(bool usetrilinear, ProgressCallback callback);

    void draw();
    void doStuff(int, bool tutorialActive, bool inDialog, AwardCallback awardNinja, AwardCallback awardBullseye, int jointstartarray[26], GameState& gamestate);

    int getType()
    {
        return type;
    }
    void setType(int);

    void drop(Vector3 velocity, Vector3 tipvelocity, bool sethitsomething = true);
    void thrown(Vector3 velocity, bool sethitsomething = true);

    int owner;
    Vector3 position;
    Vector3 tippoint;
    Vector3 velocity;
    Vector3 tipvelocity;
    bool missed;
    bool hitsomething;
    float freetime;
    bool firstfree;
    bool physics;

    float damage;
    int bloody;
    float blooddrip;
    float blooddripdelay;

    float rotation1;
    float rotation2;
    float rotation3;
    float bigrotation;
    float bigtilt;
    float bigtilt2;
    float smallrotation;
    float smallrotation2;

private:
    static Model throwingknifemodel;
    static Texture knifetextureptr;
    static Texture lightbloodknifetextureptr;
    static Texture bloodknifetextureptr;

    static Model swordmodel;
    static Texture swordtextureptr;
    static Texture lightbloodswordtextureptr;
    static Texture bloodswordtextureptr;

    static Model staffmodel;
    static Texture stafftextureptr;

    int type;

    Vector3 oldtippoint;
    Vector3 oldposition;
    int oldowner;
    bool onfire;
    float flamedelay;
    float mass;
    float tipmass;
    float length;
    float drawhowmany;

    Vector3 lastdrawnposition;
    Vector3 lastdrawntippoint;
    float lastdrawnrotation1;
    float lastdrawnrotation2;
    float lastdrawnrotation3;
    float lastdrawnbigrotation;
    float lastdrawnbigtilt;
    float lastdrawnbigtilt2;
    float lastdrawnsmallrotation;
    float lastdrawnsmallrotation2;
    int lastdrawnanim;
};

class Weapons 
{
public:
    std::vector<Weapon> weapons;

    void Draw();
    void DoStuff(bool tutorialActive, bool inDialog, int jointstartarray[26], AwardCallback awardNinja, AwardCallback awardBullseye, GameState& gamestate);
};

extern Weapons weapons;
#endif
