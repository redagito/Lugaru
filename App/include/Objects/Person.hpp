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

#ifndef _PERSON_HPP_
#define _PERSON_HPP_

#include "Objects/PersonType.hpp"
#include "Objects/Weapons.hpp"

#include "Animation/Animation.hpp"
#include "Animation/Skeleton.hpp"
#include "Environment/Terrain.hpp"
#include "Graphic/Models.hpp"
#include "Graphic/Sprite.hpp"
#include "Graphic/gamegl.hpp"

#include "Math/Vector3.hpp"

#include <cmath>
#include <memory>
#include <string>

struct GameState;

#define passivetype 0
#define guardtype 1
#define searchtype 2
#define attacktype 3
#define attacktypecutoff 4
#define playercontrolled 5
#define gethelptype 6
#define getweapontype 7
#define pathfindtype 8

struct InvalidPersonException : public std::exception
{
    const char* what() const throw()
    {
        return "Invalid weapon number";
    }
};

/**
 * Limits on the fixed-size arrays inside Person. Loaders must reject any record
 * that exceeds these before writing, otherwise they run off the end of
 * weaponids[], waypoints[] or waypointtype[].
 */
namespace PersonLimits
{
// weaponids[] holds 4 entries, indexed 0..3.
inline constexpr int max_weapons = 4;
// waypoints[] and waypointtype[] hold 90 entries.
inline constexpr int max_waypoints = 90;

inline bool weaponCountIsValid(long long count)
{
    return count >= 0 && count <= max_weapons;
}

inline bool waypointCountIsValid(long long count)
{
    return count >= 0 && count <= max_waypoints;
}
} // namespace PersonLimits

class Person : public std::enable_shared_from_this<Person>
{
private:
    float proportions[4];

public:
    static std::vector<std::shared_ptr<Person>> players;

    int whichpatchx;
    int whichpatchz;

    // animCurrent and animTarget are used to interpolate between different animations
    // (and for a bunch of other things).
    // animations interpolate with one another at various speeds.
    // animTarget seems to determine the combat state?
    int animCurrent;
    int animTarget;

    // frameCurrent and frameTarget are used to interpolate between the frames of an animation
    // (e.g. the crouched animation has only two frames, lerped back and forth slowly).
    // animations advance at various speeds.
    int frameCurrent;
    int frameTarget;

    int oldanimCurrent;
    int oldanimTarget;
    int oldframeCurrent;
    int oldframeTarget;

    int howactive;

    float parriedrecently;

    bool superruntoggle;

    int lastattack, lastattack2, lastattack3;

    Vector3 currentoffset, targetoffset, offset;
    float target;
    float transspeed;

    Vector3 realoldcoords;
    Vector3 oldcoords;
    Vector3 coords;
    Vector3 velocity;

    float unconscioustime;

    bool immobile;

    float velspeed;
    float targetyaw;
    float targetrot;
    float rot;
    float oldrot;
    float lookyaw;
    float lookpitch;
    float yaw;
    float pitch;
    float lowyaw;
    float tilt;
    float targettilt;
    float tilt2;
    float targettilt2;
    bool rabbitkickenabled;

    float bloodloss;
    float bleeddelay;
    float skiddelay;
    float skiddingdelay;
    float deathbleeding;
    float tempdeltav;

    float damagetolerance;
    float damage;
    float permanentdamage;
    float superpermanentdamage;
    float lastcollide;
    /* Seems to be 0 = alive, 1 = unconscious, 2 = dead */
    int dead;

    float jumppower;
    bool onground;

    int wentforweapon;

    bool calcrot;

    Vector3 facing;

    float bleeding;
    float bleedx, bleedy;
    int direction;
    float texupdatedelay;

    float headyaw, headpitch;
    float targetheadyaw, targetheadpitch;

    bool onterrain;
    bool pause;

    float grabdelay;

    std::shared_ptr<Person> victim;
    bool hasvictim;

    float updatedelay;
    float normalsupdatedelay;

    bool jumpstart;

    bool forwardkeydown;
    bool forwardstogglekeydown;
    bool rightkeydown;
    bool leftkeydown;
    bool backkeydown;
    bool jumpkeydown;
    bool jumptogglekeydown;
    bool crouchkeydown;
    bool crouchtogglekeydown;
    bool drawkeydown;
    bool drawtogglekeydown;
    bool throwkeydown;
    bool throwtogglekeydown;
    bool attackkeydown;
    bool feint;
    bool lastfeint;
    bool headless;

    float crouchkeydowntime;
    float jumpkeydowntime;
    bool freefall;

    float turnspeed;

    int aitype;
    float aiupdatedelay;
    float losupdatedelay;
    int ally;
    float collide;
    float collided;
    float avoidcollided;
    bool loaded;
    bool whichdirection;
    float whichdirectiondelay;
    bool avoidsomething;
    Vector3 avoidwhere;
    float blooddimamount;

    float staggerdelay;
    float blinkdelay;
    float twitchdelay;
    float twitchdelay2;
    float twitchdelay3;
    float lefthandmorphness;
    float righthandmorphness;
    float headmorphness;
    float chestmorphness;
    float tailmorphness;
    float targetlefthandmorphness;
    float targetrighthandmorphness;
    float targetheadmorphness;
    float targetchestmorphness;
    float targettailmorphness;
    int lefthandmorphstart, lefthandmorphend;
    int righthandmorphstart, righthandmorphend;
    int headmorphstart, headmorphend;
    int chestmorphstart, chestmorphend;
    int tailmorphstart, tailmorphend;

    float weaponmissdelay;
    float highreversaldelay;
    float lowreversaldelay;

    int creature;

    unsigned id;

    Skeleton skeleton;

    float speed;
    float scale;
    float power;
    float speedmult;

    float protectionhead;
    float protectionhigh;
    float protectionlow;
    float armorhead;
    float armorhigh;
    float armorlow;
    bool metalhead;
    bool metalhigh;
    bool metallow;

    std::vector<std::string> clothes;
    std::vector<float> clothestintr;
    std::vector<float> clothestintg;
    std::vector<float> clothestintb;

    bool landhard;
    bool bled;
    bool spurt;
    bool onfire;
    float onfiredelay;
    float burnt;

    float flamedelay;

    int playerdetail;

    int num_weapons;
    int weaponids[PersonLimits::max_weapons];
    /* Key of weaponids which is the weapon in hand, if any. -1 otherwise.
     * Always 0 or -1 as activeweapon is moved to position 0 when taken */
    int weaponactive;
    int weaponstuck;
    /* 0 or 1 to say if weapon is stuck in the front or the back  */
    int weaponstuckwhere;

    int numwaypoints;
    Vector3 waypoints[PersonLimits::max_waypoints];
    int waypointtype[PersonLimits::max_waypoints];
    float pausetime;

    Vector3 headtarget;
    float interestdelay;

    Vector3 finalfinaltarget;
    Vector3 finaltarget;
    int finalpathfindpoint;
    int targetpathfindpoint;
    int lastpathfindpoint;
    int lastpathfindpoint2;
    int lastpathfindpoint3;
    int lastpathfindpoint4;

    int waypoint;

    Vector3 lastseen;
    float lastseentime;
    float lastchecktime;
    float stunned;
    float surprised;
    float runninghowlong;
    int occluded;
    int lastoccluded;
    int laststanding;
    int escapednum;

    float speechdelay;
    float neckspurtdelay;
    float neckspurtparticledelay;
    float neckspurtamount;

    int whichskin;
    bool rabbitkickragdoll;

    Animation tempanimation;

    bool jumpclimb;

    Person(GameState& gamestate);
    Person(FILE*, int, unsigned, GameState& gamestate);
    Person(Json::Value, int, unsigned, GameState& gamestate);

    void skeletonLoad(bool tutorialActive, GameState& gamestate);

    // convenience functions
    inline Joint& joint(int bodypart) { return skeleton.joints[skeleton.jointlabels[bodypart]]; }
    inline Vector3& jointPos(int bodypart) { return joint(bodypart).position; }
    inline Vector3& jointVel(int bodypart) { return joint(bodypart).velocity; }
    inline AnimationFrame& currentFrame() { return Animation::animations.at(animCurrent).frames.at(frameCurrent); }
    inline AnimationFrame& targetFrame() { return Animation::animations.at(animTarget).frames.at(frameTarget); }

    void setProportions(float head, float body, float arms, float legs);
    float getProportion(int part) const;
    Vector3 getProportionXYZ(int part, GameState& gamestate) const;

    void changeCreatureType(person_type type, bool tutorialActive, GameState& gamestate);

    void CheckKick(Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate);
    void CatchFire(GameState& gamestate);
    void DoBlood(float howmuch, int which, bool tutorialActive, GameState& gamestate);
    void DoBloodBig(float howmuch, int which, bool tutorialActive, GameState& gamestate);
    bool DoBloodBigWhere(float howmuch, int which, Vector3 where, bool tutorialActive, GameState& gamestate);

    bool wasIdle() { return animation_bits[animCurrent] & ab_idle; }
    bool isIdle() { return animation_bits[animTarget] & ab_idle; }
    int getIdle(bool inDialog);

    bool isSitting() { return animation_bits[animTarget] & ab_sit; }

    bool isSleeping() { return animation_bits[animTarget] & ab_sleep; }

    bool wasCrouch() { return animation_bits[animCurrent] & ab_crouch; }
    bool isCrouch() { return animation_bits[animTarget] & ab_crouch; }
    int getCrouch();

    bool wasStop() { return animation_bits[animCurrent] & ab_stop; }
    bool isStop() { return animation_bits[animTarget] & ab_stop; }
    int getStop();

    bool wasRun() const { return animation_bits[animCurrent] & ab_run; }
    bool isRun() const { return animation_bits[animTarget] & ab_run; }
    int getRun();

    /** True when this character should steer its yaw toward `targetyaw` this frame. */
    bool shouldTurnTowardTarget() const;

    bool wasLanding() { return animation_bits[animCurrent] & ab_land; }
    bool isLanding() { return animation_bits[animTarget] & ab_land; }
    int getLanding();

    bool wasLandhard() { return animation_bits[animCurrent] & ab_landhard; }
    bool isLandhard() { return animation_bits[animTarget] & ab_landhard; }
    int getLandhard();

    bool wasFlip() { return animation_bits[animCurrent] & ab_flip; }
    bool isFlip() { return animation_bits[animTarget] & ab_flip; }

    bool isWallJump() { return animation_bits[animTarget] & ab_walljump; }
    void Reverse(bool tutorialActive, GameState& gamestate);
    void DoDamage(float howmuch, Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate);
    void DoHead(float timemultiplier, GameState& gamestate);
    void DoMipmaps()
    {
        skeleton.drawmodel.textureptr.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, skeleton.skinsize, skeleton.skinsize, 0, GL_RGB, GL_UNSIGNED_BYTE, &skeleton.skinText[0]);
    }

    int SphereCheck(Vector3* p1, float radius, Vector3* p, Vector3* move, float* rotate, Model* model, Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate);
    int DrawSkeleton(Terrain& terrain, bool tutorialActive, float timemultiplier, int whichjointstartarray[26], GameState& gamestate);
    void Puff(int whichlabel, GameState& gamestate);
    void FootLand(bodypart whichfoot, float opacity, Terrain& terrain, GameState& gamestate);
    void DoStuff(Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate);
    void setTargetAnimation(int);
    void DoAnimations(Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate);
    void RagDoll(bool checkcollision, Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate);

    void takeWeapon(int weaponId);

    bool addClothes(const int& clothesId, GameState& gamestate);
    void addClothes(GameState& gamestate);

    void doAI(const Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, GameState& gamestate);

    bool catchKnife();

    bool hasWeapon() { return (weaponactive != -1); }
    bool isPlayerControlled() { return (aitype == playercontrolled); }

    operator Json::Value();
};

const int maxplayers = 10;

#endif
