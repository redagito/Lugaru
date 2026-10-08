#pragma once

// The last two globals this header used to declare, terrain and weapons, now
// live in GameAssets and are reached through it. The includes stay: other
// translation units still rely on this header pulling them in.

#include "Graphic/Stereo.hpp"
#include "Math/Vector3.hpp"
#include "Objects/Weapons.hpp"

#include <SDL.h>
