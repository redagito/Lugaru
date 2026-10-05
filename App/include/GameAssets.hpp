#pragma once

#include "Environment/Skybox.hpp"
#include "Graphic/Models.hpp"
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
//
// The four models were externs in GameGlobals.h for the same reason the textures
// were. A Model owns four malloc'd buffers through raw pointers and frees them in
// its destructor, so it cannot live in GameState either: that would break the
// trivial copyability and trivial destructibility GameState is asserted to have,
// and the implicit copy would free the same buffer twice. Declared last, so they
// are destroyed first and leave the GL-touching members above undisturbed.

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

	Model hawk;
	Model eye;
	Model cornea;
	Model iris;
};