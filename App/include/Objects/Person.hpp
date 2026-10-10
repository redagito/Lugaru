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
#include "GameAssets.hpp"
#include "Graphic/Models.hpp"
#include "Graphic/Sprite.hpp"
#include "Graphic/gamegl.hpp"

#include "Math/Vector3.hpp"

#include <cmath>
#include <memory>
#include <string>

struct GameAssets;
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
    const char* what() const throw();
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

bool weaponCountIsValid(long long count);
bool waypointCountIsValid(long long count);
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

    Person(GameState& gamestate, GameAssets& assets);
    Person(FILE*, int, unsigned, GameState& gamestate, GameAssets& assets);
    Person(Json::Value, int, unsigned, GameState& gamestate, GameAssets& assets);

    void skeletonLoad(bool tutorialActive, GameState& gamestate, GameAssets& assets);

    // convenience functions
    Joint& joint(int bodypart);
    Vector3& jointPos(int bodypart);
    Vector3& jointVel(int bodypart);
    AnimationFrame& currentFrame(GameAssets& assets);
    AnimationFrame& targetFrame(GameAssets& assets);

    void setProportions(float head, float body, float arms, float legs);
    float getProportion(int part) const;
    Vector3 getProportionXYZ(int part, GameState& gamestate, GameAssets& assets) const;

    void changeCreatureType(person_type type, bool tutorialActive, GameState& gamestate, GameAssets& assets);

    void CheckKick(Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate, GameAssets& assets);
    void CatchFire(GameState& gamestate);
    void DoBlood(float howmuch, int which, bool tutorialActive, GameState& gamestate, GameAssets& assets);
    void DoBloodBig(float howmuch, int which, bool tutorialActive, GameState& gamestate, GameAssets& assets);
    bool DoBloodBigWhere(float howmuch, int which, Vector3 where, bool tutorialActive, GameState& gamestate, GameAssets& assets);

    bool wasIdle();
    bool isIdle();
    int getIdle(bool inDialog, GameAssets& assets);

    bool isSitting();

    bool isSleeping();

    bool wasCrouch();
    bool isCrouch();
    int getCrouch(GameAssets& assets);

    bool wasStop();
    bool isStop();
    int getStop(GameAssets& assets);

    bool wasRun() const;
    bool isRun() const;
    int getRun(GameAssets& assets);

    /** True when this character should steer its yaw toward `targetyaw` this frame. */
    bool shouldTurnTowardTarget() const;

    bool wasLanding();
    bool isLanding();
    int getLanding(GameAssets& assets);

    bool wasLandhard();
    bool isLandhard();
    int getLandhard(GameAssets& assets);

    bool wasFlip();
    bool isFlip();

    bool isWallJump();
    void Reverse(bool tutorialActive, GameState& gamestate, GameAssets& assets);
    void DoDamage(float howmuch, Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate, GameAssets& assets);
    void DoHead(float timemultiplier, GameState& gamestate);
    void DoMipmaps();

    int SphereCheck(Vector3* p1, float radius, Vector3* p, Vector3* move, float* rotate, Model* model, Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate, GameAssets& assets);
    int DrawSkeleton(Terrain& terrain, bool tutorialActive, float timemultiplier, int whichjointstartarray[26], GameState& gamestate, GameAssets& assets);
    void Puff(int whichlabel, GameState& gamestate);
    void FootLand(bodypart whichfoot, float opacity, Terrain& terrain, GameState& gamestate);
    void DoStuff(Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate, GameAssets& assets);
    void setTargetAnimation(int);
    void DoAnimations(Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate, GameAssets& assets);
    void RagDoll(bool checkcollision, Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, int whichjointstartarray[26], GameState& gamestate, GameAssets& assets);

    void takeWeapon(int weaponId, GameAssets& assets);

    bool addClothes(const int& clothesId, GameState& gamestate, GameAssets& assets);
    void addClothes(GameState& gamestate, GameAssets& assets);

    void doAI(const Terrain& terrain, bool tutorialActive, bool inDialog, float timemultiplier, GameState& gamestate, GameAssets& assets);

    bool catchKnife(GameAssets& assets);

    bool hasWeapon();
    bool isPlayerControlled();

    // Was `operator Json::Value()`. A conversion operator takes no parameters,
    // and the weapon types it has to write out live in the GameAssets weapons, so
    // it cannot reach them. Nothing called it; the named form can.
    Json::Value save(GameAssets& assets);
};

const int maxplayers = 10;

#endif
