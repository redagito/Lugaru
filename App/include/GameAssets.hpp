#pragma once

#include "Environment/Skybox.hpp"
#include "Graphic/Text.hpp"
#include "Graphic/Texture.hpp"

// GameAssets owns the session-long rendering helpers that hold OpenGL handles:
// the skybox, the two fonts and the shared textures. They cannot live in
// GameState, which has to stay trivially copyable so it can be built and
// destroyed in a unit test with no GL context, so they get an owner of their
// own.
//
// Held by value and passed by reference, from the construction in main() down to
// the load and draw paths. That is the whole point: the three objects used to be
// raw new/delete pairs behind extern pointers, which tied nothing to the release
// and leaked everything on any early return. By value there is nothing to leak.
// The textures were eleven externs in GameGlobals.h; a Texture is a shared_ptr to
// a GL object, so the handle was reachable from any file with no owner in sight.
//
// ~Text and ~TextureRes both call into GL, so an instance has to be destroyed
// while its GL context is still current. In main() that means it must go out of
// scope before SDL_Quit, which it does.

struct GameAssets
{
	SkyBox skybox;
	Text text;
	Text textmono;

	Texture terraintexture;
	Texture terraintexture2;
	Texture loadscreentexture;
	Texture Mapcircletexture;
	Texture Maparrowtexture;
	Texture Mapboxtexture;
	Texture cursortexture;
	Texture Mainmenuitems[10];
	Texture hawktexture;
	GLuint screentexture = 0;
	GLuint screentexture2 = 0;
};