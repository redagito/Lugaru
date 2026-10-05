#pragma once

#include "Graphic/Stereo.hpp"
#include "Math/Vector3.hpp"
#include "Objects/Weapons.hpp"

#include <SDL.h>

extern Vector3 viewer;
extern Vector3 viewerfacing;
extern Light light;
extern Terrain terrain;
extern SDL_Window* sdlwindow;
extern Frustum frustum;
extern Weapons weapons;
extern Vector3 windvector;
extern StereoMode stereomode;
extern StereoMode newstereomode;
