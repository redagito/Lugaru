#pragma once

#include "Environment/Skybox.hpp"
#include "Environment/Terrain.hpp"
#include "Graphic/Models.hpp"
#include "Graphic/Text.hpp"
#include "Graphic/Texture.hpp"
#include "GraphicsState.hpp"
#include "Objects/Weapons.hpp"

#include <memory>

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
//
// The terrain and the weapons were the last two externs in Globals.h. Neither
// can live in GameState, for the same reason the models cannot: GameState is
// asserted to be trivially copyable and trivially destructible. The two shapes
// differ, and neither is free:
//
//  - The terrain's ~22 MB of fixed arrays were why it sat behind a pointer:
//    GameAssets is a stack local, so an inline member made it overflow the
//    stack. Those arrays are heap blocks owned by Terrain itself now, so a
//    Terrain is 424 bytes and a value would cost nothing here too. It stays a
//    pointer until the remaining terrain members are worth the churn.
//  - A weapons is 24 bytes plus a heap vector, so a value costs nothing.
//
// Both were globals whose destructors ran after main returned, which is after
// SDL_Quit. As members they are destroyed with the rest of GameAssets, while
// the GL context is still current.

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

	// sprite textures
	Texture cloudtexture;
	Texture cloudimpacttexture;
	Texture bloodtexture;
	Texture flametexture;
	Texture bloodflametexture;
	Texture smoketexture;
	Texture snowflaketexture;
	Texture shinetexture;
	Texture splintertexture;
	Texture leaftexture;
	Texture toothtexture;

	// object textures
	Texture boxtextureptr;
	Texture treetextureptr;
	Texture bushtextureptr;
	Texture rocktextureptr;

	GLuint screentexture = 0;
	GLuint screentexture2 = 0;

	Model hawk;
	Model eye;
	Model cornea;
	Model iris;

	// Neither an Animation nor a PersonType owns an OpenGL handle: their
	// members are enums, Vector3s, std::strings, ints, floats and plain
	// vectors, so the Graphics state they live in is not tied to a live GL
	// context either. It therefore needs no placement rule against the
	// GL-touching members above and is simply declared after them.
	GraphicsState graphics;

	std::unique_ptr<Terrain> terrain;
	Weapons weapons;

	// A Model frees its own buffers, so the copy the compiler would generate here
	// would hand two instances the same four pointers and both destructors would
	// free them. Both live instances are locals passed by reference, so nothing
	// copies one; forbidding the copy turns the first attempt into a compile error
	// instead of heap corruption.
	GameAssets() = default;
	GameAssets(const GameAssets&) = delete;
	GameAssets& operator=(const GameAssets&) = delete;
};