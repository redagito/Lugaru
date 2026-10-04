#pragma once

#include "Graphic/Stereo.hpp"
#include "Math/Vector3.hpp"
#include "Objects/Weapons.hpp"

#include <SDL.h>

extern float volume;
extern bool ismotionblur;
extern float usermousesensitivity;
extern bool decalstoggle;
extern float blurness;
extern float windvar;
extern int difficulty;
extern float multiplier;
extern float screenwidth;
extern float screenheight;
extern bool fullscreen;
extern float viewdistance;
extern Vector3 viewer;
extern Vector3 viewerfacing;
extern float fadestart;
extern int environment;
extern float texscale;
extern float gravity;
extern Light light;
extern Terrain terrain;
extern SDL_Window* sdlwindow;
extern int kTextureSize;
extern int detail;
extern Frustum frustum;
extern float texdetail;
extern float realtexdetail;
extern float playerdist;
extern int slomo;
extern int bloodtoggle;
extern float camerashake;
extern float woozy;
extern float blackout;
extern bool musictoggle;
extern bool trilinear;
extern Weapons weapons;
extern bool ambientsound;
extern float flashamount;
extern Vector3 windvector;
extern int mainmenu;
extern int whichjointstartarray[26];
extern int whichjointendarray[26];
extern float smoketex;
extern int maptype;
extern bool reversaltrain;
extern bool canattack;
extern bool skyboxtexture;
extern float skyboxr;
extern float skyboxg;
extern float skyboxb;
extern float skyboxlightr;
extern float skyboxlightg;
extern float skyboxlightb;
extern int hostile;
extern bool devtools;
extern bool gamestarted;
extern StereoMode stereomode;
extern StereoMode newstereomode;
extern float stereoseparation;
