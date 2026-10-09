/*
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

#ifndef _ANIMATION_HPP_
#define _ANIMATION_HPP_

#include "Animation/AnimationDefinitions.h"

#include "Math/Vector3.hpp"
#include "Utils/Callbacks.h"

#include <vector>

struct GraphicsState;

struct AnimationFrameJointInfo
{
    Vector3 position;
    float twist = 0.f;
    float twist2 = 0.f;
    bool onground = false;
};

struct AnimationFrame
{
    void loadBaseInfo(FILE* tfile);
    void loadTwist2(FILE* tfile);
    void loadLabel(FILE* tfile);
    void loadWeaponTarget(FILE* tfile);

    std::vector<AnimationFrameJointInfo> joints;
    Vector3 forward;
    int label;
    Vector3 weapontarget;
    float speed;
};

class Animation
{
public:
    static void loadAll(ProgressCallback callback, GraphicsState& graphics);

    anim_height_type height;
    anim_attack_type attack;
    int numjoints;

    std::vector<AnimationFrame> frames;

    Vector3 offset;

    Animation();
    Animation(const std::string& fileName, anim_height_type aheight, anim_attack_type aattack, ProgressCallback callback);
};
#endif
