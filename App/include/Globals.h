#pragma once

#include "Graphic/Stereo.hpp"
#include "Math/Vector3.hpp"
#include "Objects/Weapons.hpp"

#include <SDL.h>

extern bool decalstoggle;
extern float blurness;
extern float windvar;
extern int difficulty;
extern float multiplier;
extern float screenwidth;
extern float screenheight;
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
extern int detail;
extern Frustum frustum;
extern float texdetail;
extern float realtexdetail;
extern int slomo;
extern int bloodtoggle;
extern float camerashake;
extern float blackout;
extern bool trilinear;
extern Weapons weapons;
extern float flashamount;
extern Vector3 windvector;
extern int mainmenu;
extern int whichjointstartarray[26];
extern int whichjointendarray[26];
extern bool skyboxtexture;
extern int hostile;
extern bool devtools;
extern StereoMode stereomode;
extern StereoMode newstereomode;
