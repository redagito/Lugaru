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

#include "Objects/PersonType.hpp"

#include "GraphicsState.hpp"

PersonType::PersonType()
{
    animTalkIdle = tempanim;
    animHurtIdle = tempanim;
}

bool PersonType::hasAnimTalkIdle()
{
    return (animTalkIdle != tempanim);
}

bool PersonType::hasAnimHurtIdle()
{
    return (animHurtIdle != tempanim);
}

void PersonType::Load(GraphicsState& graphics)
{
    graphics.types.resize(2);

    /* Wolf */
    graphics.types[wolftype].proportions[0] = 1.1f;
    graphics.types[wolftype].proportions[1] = 1.1f;
    graphics.types[wolftype].proportions[2] = 1.1f;
    graphics.types[wolftype].proportions[3] = 1.1f;

    graphics.types[wolftype].animRun = wolfrunanim;
    graphics.types[wolftype].animRunning = wolfrunninganim;
    graphics.types[wolftype].animCrouch = wolfcrouchanim;
    graphics.types[wolftype].animStop = wolfstopanim;
    graphics.types[wolftype].animLanding = wolflandanim;
    graphics.types[wolftype].animLandingHard = wolflandhardanim;
    graphics.types[wolftype].animFightIdle = wolfidle;
    graphics.types[wolftype].animBounceIdle = wolfidle;

    graphics.types[wolftype].soundsAttack[0] = barksound;
    graphics.types[wolftype].soundsAttack[1] = bark2sound;
    graphics.types[wolftype].soundsAttack[2] = bark3sound;
    graphics.types[wolftype].soundsAttack[3] = barkgrowlsound;
    graphics.types[wolftype].soundsTalk[0] = growlsound;
    graphics.types[wolftype].soundsTalk[1] = growl2sound;

    graphics.types[wolftype].figureFileName = "Skeleton/BasicFigureWolf";
    graphics.types[wolftype].lowFigureFileName = "Skeleton/BasicFigureWolfLow";
    graphics.types[wolftype].clothesFileName = "Skeleton/RabbitBelt";
    graphics.types[wolftype].modelFileNames[0] = "Models/Wolf.solid";
    graphics.types[wolftype].modelFileNames[1] = "Models/Wolf2.solid";
    graphics.types[wolftype].modelFileNames[2] = "Models/Wolf3.solid";
    graphics.types[wolftype].modelFileNames[3] = "Models/Wolf4.solid";
    graphics.types[wolftype].modelFileNames[4] = "Models/Wolf5.solid";
    graphics.types[wolftype].modelFileNames[5] = "Models/Wolf6.solid";
    graphics.types[wolftype].modelFileNames[6] = "Models/Wolf7.solid";
    graphics.types[wolftype].lowModelFileName = "Models/WolfLow.solid";
    graphics.types[wolftype].modelClothesFileName = "Models/Belt.solid";

    graphics.types[wolftype].skins.resize(3);
    graphics.types[wolftype].skins[0] = "Textures/FurWolfGrey.jpg";
    graphics.types[wolftype].skins[1] = "Textures/FurWolfDark.jpg";
    graphics.types[wolftype].skins[2] = "Textures/FurWolfSnow.jpg";

    graphics.types[wolftype].power = 2.5f;
    graphics.types[wolftype].defaultDamageTolerance = 300;
    graphics.types[wolftype].defaultScale = .23f;
    graphics.types[wolftype].hasClaws = true;
    graphics.types[wolftype].clothes = false;
    graphics.types[wolftype].maxRunSpeed = 75;
    graphics.types[wolftype].knifeCatchingType = 1;

    /* Rabbit */
    graphics.types[rabbittype].proportions[0] = 1.2f;
    graphics.types[rabbittype].proportions[1] = 1.05f;
    graphics.types[rabbittype].proportions[2] = 1;
    graphics.types[rabbittype].proportions[3] = 1.1f;
    graphics.types[rabbittype].proportions[3].y = 1.05f;

    graphics.types[rabbittype].animRun = runanim;
    graphics.types[rabbittype].animRunning = rabbitrunninganim;
    graphics.types[rabbittype].animCrouch = crouchanim;
    graphics.types[rabbittype].animStop = stopanim;
    graphics.types[rabbittype].animLanding = landanim;
    graphics.types[rabbittype].animLandingHard = landhardanim;
    graphics.types[rabbittype].animFightIdle = fightidleanim;
    graphics.types[rabbittype].animBounceIdle = bounceidleanim;
    graphics.types[rabbittype].animTalkIdle = talkidleanim;
    graphics.types[rabbittype].animHurtIdle = hurtidleanim;

    graphics.types[rabbittype].soundsAttack[0] = rabbitattacksound;
    graphics.types[rabbittype].soundsAttack[1] = rabbitattack2sound;
    graphics.types[rabbittype].soundsAttack[2] = rabbitattack3sound;
    graphics.types[rabbittype].soundsAttack[3] = rabbitattack4sound;
    graphics.types[rabbittype].soundsTalk[0] = rabbitchitter;
    graphics.types[rabbittype].soundsTalk[1] = rabbitchitter2;

    graphics.types[rabbittype].figureFileName = "Skeleton/BasicFigure";
    graphics.types[rabbittype].lowFigureFileName = "Skeleton/BasicFigureLow";
    graphics.types[rabbittype].clothesFileName = "Skeleton/RabbitBelt";
    graphics.types[rabbittype].modelFileNames[0] = "Models/Body.solid";
    graphics.types[rabbittype].modelFileNames[1] = "Models/Body2.solid";
    graphics.types[rabbittype].modelFileNames[2] = "Models/Body3.solid";
    graphics.types[rabbittype].modelFileNames[3] = "Models/Body4.solid";
    graphics.types[rabbittype].modelFileNames[4] = "Models/Body5.solid";
    graphics.types[rabbittype].modelFileNames[5] = "Models/Body6.solid";
    graphics.types[rabbittype].modelFileNames[6] = "Models/Body7.solid";
    graphics.types[rabbittype].lowModelFileName = "Models/BodyLow.solid";
    graphics.types[rabbittype].modelClothesFileName = "Models/Belt.solid";

    graphics.types[rabbittype].skins.resize(10);
    graphics.types[rabbittype].skins[0] = "Textures/FurBrown.jpg";
    graphics.types[rabbittype].skins[1] = "Textures/FurWhite.jpg";
    graphics.types[rabbittype].skins[2] = "Textures/FurBlack.jpg";
    graphics.types[rabbittype].skins[3] = "Textures/FurLynx.jpg";
    graphics.types[rabbittype].skins[4] = "Textures/FurOtter.jpg";
    graphics.types[rabbittype].skins[5] = "Textures/FurOpal.jpg";
    graphics.types[rabbittype].skins[6] = "Textures/FurSable.jpg";
    graphics.types[rabbittype].skins[7] = "Textures/FurChocolate.jpg";
    graphics.types[rabbittype].skins[8] = "Textures/FurBlackWhite.jpg";
    graphics.types[rabbittype].skins[9] = "Textures/FurBrownWhite.jpg";

    graphics.types[rabbittype].power = 1;
    graphics.types[rabbittype].defaultDamageTolerance = 200;
    graphics.types[rabbittype].defaultScale = .2f;
    graphics.types[rabbittype].hasClaws = false;
    graphics.types[rabbittype].clothes = true;
    graphics.types[rabbittype].maxRunSpeed = 55;
    graphics.types[rabbittype].knifeCatchingType = 0;
}
