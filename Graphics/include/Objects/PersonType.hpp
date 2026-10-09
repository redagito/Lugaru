/*
Copyright (C) 2016-2017 - Lugaru contributors (see AUTHORS file)

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

#ifndef _PERSONTYPE_HPP_
#define _PERSONTYPE_HPP_

#include "Animation/AnimationDefinitions.h"
#include "Audio/Sounds.hpp"
#include "Math/Vector3.hpp"

enum person_type
{
    rabbittype = 0,
    wolftype = 1
};

struct GraphicsState;

class PersonType
{
public:
    // head, body, arms, legs
    Vector3 proportions[4];
    animation_type animRun;
    animation_type animRunning;
    animation_type animCrouch;
    animation_type animStop;
    animation_type animLanding;
    animation_type animLandingHard;
    animation_type animFightIdle;
    animation_type animBounceIdle;
    animation_type animTalkIdle;
    animation_type animHurtIdle;

    sound_type soundsAttack[4];
    sound_type soundsTalk[2];

    std::string figureFileName;
    std::string lowFigureFileName;
    std::string clothesFileName;
    std::string modelFileNames[7];
    std::string lowModelFileName;
    std::string modelClothesFileName;

    std::vector<std::string> skins;

    float power;
    unsigned int defaultDamageTolerance = 0;
    float defaultScale = 1.f;
    bool hasClaws = false;
    bool clothes = false;
    unsigned int maxRunSpeed = 0;
    unsigned int knifeCatchingType = 0;

    uint8_t bloodText[512 * 512 * 3] = { 0 };

    PersonType();
    bool hasAnimTalkIdle();
    bool hasAnimHurtIdle();

    static void Load(GraphicsState& graphics);
};

#endif
