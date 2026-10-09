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

#include "Animation/Animation.hpp"

#include "GraphicsState.hpp"

#include "Utils/binio.h"
#include "Utils/Folders.hpp"

void Animation::loadAll(ProgressCallback callback, GraphicsState& graphics)
{
#define DECLARE_ANIM(id, file, height, attack, ...) \
    if constexpr (id < loadable_anim_end)                     \
        graphics.animations.emplace_back(file, height, attack, callback);
#include "Animation/Animation.inc"
#undef DECLARE_ANIM
}

void AnimationFrame::loadBaseInfo(FILE* tfile)
{
    // for each joint in the skeleton...
    for (unsigned j = 0; j < joints.size(); j++) {
        // read joint position
        funpackf(tfile, "Bf Bf Bf", &joints[j].position.x, &joints[j].position.y, &joints[j].position.z);
    }
    for (unsigned j = 0; j < joints.size(); j++) {
        // read twist
        funpackf(tfile, "Bf", &joints[j].twist);
    }
    for (unsigned j = 0; j < joints.size(); j++) {
        // read onground (boolean)
        unsigned char uch;
        funpackf(tfile, "Bb", &uch);
        joints[j].onground = (uch != 0);
    }
    // read frame speed (?)
    funpackf(tfile, "Bf", &speed);
}

void AnimationFrame::loadTwist2(FILE* tfile)
{
    for (unsigned j = 0; j < joints.size(); j++) {
        funpackf(tfile, "Bf", &joints[j].twist2);
    }
}

void AnimationFrame::loadLabel(FILE* tfile)
{
    funpackf(tfile, "Bf", &label);
}

void AnimationFrame::loadWeaponTarget(FILE* tfile)
{
    funpackf(tfile, "Bf Bf Bf", &weapontarget.x, &weapontarget.y, &weapontarget.z);
}

Animation::Animation()
    : height(lowheight)
    , attack(neutral)
    , numjoints(0)
{
}

/* EFFECT
 * load an animation from file
 */
Animation::Animation(const std::string& filename, anim_height_type aheight, anim_attack_type aattack, ProgressCallback callback)
    : Animation()
{
    FILE* tfile;
    int numframes;
    unsigned i;

    // Changing the filename into something the OS can understand
    std::string filepath = Folders::getResourcePath("Animations/" + filename);

    height = aheight;
    attack = aattack;

    //Game::LoadingScreen();
    callback();

    // read file in binary mode
    tfile = Folders::openMandatoryFile(filepath, "rb");

    // read numframes, joints to know how much memory to allocate
    funpackf(tfile, "Bi Bi", &numframes, &numjoints);

    // allocate memory for everything

    frames.resize(numframes);

    // read binary data as animation

    // for each frame...
    for (i = 0; i < frames.size(); i++) {
        frames[i].joints.resize(numjoints);
        frames[i].loadBaseInfo(tfile);
    }
    // read twist2 for whole animation
    for (i = 0; i < frames.size(); i++) {
        frames[i].loadTwist2(tfile);
    }
    // read label for each frame
    for (i = 0; i < frames.size(); i++) {
        frames[i].loadLabel(tfile);
    }
    // read unused weapontargetnum
    // The trailing weapontarget block is optional: animation files written
    // before that feature was added simply end after the labels. Read it
    // leniently and fall back to the default (zeroed) weapon targets.
    int weapontargetnum = 0;
    if (tryfunpackf(tfile, "Bi", &weapontargetnum)) {
        // read weapontarget positions for each frame
        for (i = 0; i < frames.size(); i++) {
            if (!tryfunpackf(tfile, "Bf Bf Bf", &frames[i].weapontarget.x, &frames[i].weapontarget.y, &frames[i].weapontarget.z)) {
                break;
            }
        }
    }

    fclose(tfile);

    Vector3 endoffset;
    endoffset = 0;
    // find average position of certain joints on last frames
    // and save in endoffset
    // (not sure what exactly this accomplishes. the y < 1 test confuses me.)
    for (i = 0; i < frames.back().joints.size(); i++) {
        if (frames.back().joints[i].position.y < 1) {
            endoffset += frames.back().joints[i].position;
        }
    }
    endoffset /= (float)numjoints;
    offset = endoffset;
    offset.y = 0;
}
