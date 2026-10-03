#pragma once

// GameState replaces globals previously declared in App/include/Globals.h and
// App/include/GameGlobals.h. It is expected to be constructed once and passed
// by reference; it owns no storage outside itself, so two instances never
// share values.
//
// Scalars only. Rendering resources (Texture, Model) deliberately stay out:
// they need a GL context, so putting them here would make this struct
// untestable outside the renderer and non-trivially destructible.

struct GameState
{
	// editor
	int editoractive = 0;
	int editorpathtype = 0;

	// hawk
	float hawkyaw = 0;
	float hawkcalldelay = 0;

	// cursor
	int oldmousecoordv = 0;

	// console
	float consoleblinkdelay = 0;
	bool consoleblink = false;

	// screen limits
	float maxscreenwidth = 3000;
	float maxscreenheight = 3000;

	// input
	bool mousejump = false;

	// scoring
	bool scoreadded = false;
	bool againbonus = false;

	// timing
	float slomospeed = 0;
	float fps = 0;
};

// TEMPORARY SEAM. state() exists so the migration away from globals is a small,
// reviewable diff; it is a single global by another name and later phases must
// replace it with an explicitly passed GameState reference.
GameState& state();