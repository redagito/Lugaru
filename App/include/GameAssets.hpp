#pragma once

#include "Environment/Skybox.hpp"
#include "Graphic/Text.hpp"

// GameAssets owns the session-long rendering helpers that hold OpenGL handles:
// the skybox and the two fonts. They cannot live in GameState, which has to
// stay trivially copyable so it can be built and destroyed in a unit test with
// no GL context, so they get an owner of their own.
//
// Held by value and passed by reference, from the construction in main() down to
// the load and draw paths. That is the whole point: the three used to be raw
// new/delete pairs behind extern pointers, which tied nothing to the release
// and leaked everything on any early return. By value there is nothing to leak.
//
// ~Text calls glDeleteLists, so an instance has to be destroyed while its GL
// context is still current. In main() that means it must go out of scope before
// SDL_Quit, which it does.

struct GameAssets
{
	SkyBox skybox;
	Text text;
	Text textmono;
};