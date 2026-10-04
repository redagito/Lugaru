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
	float editorsize = 0;

	// hawk
	float hawkyaw = 0;
	float hawkcalldelay = 0;

	// console
	float consoleblinkdelay = 0;
	bool consoleblink = false;
	unsigned consoleselected = 0;
	unsigned short consolekey = 0;

	// screen limits
	float maxscreenwidth = 3000;
	float maxscreenheight = 3000;
	float minscreenwidth = 640;
	float minscreenheight = 480;

	// input
	bool mousejump = false;
	bool floatjump = false;
	bool invertmouse = false;
	int mousecoordh = 0;
	int mousecoordv = 0;

	// scoring
	bool scoreadded = false;
	bool againbonus = false;
	float damagedealt = 0;

	// timing
	float slomospeed = 0;
	float fps = 0;
	float realmultiplier = 0;
	float hostiletime = 0;

	// campaign choice
	int whichchoice = 0;

	// time scale
	float gamespeed = 0;
	float oldgamespeed = 0;
	float slomodelay = 0;
	bool autoslomo = false;

	// level loading
	int loading = 0;
	bool stillloading = false;
	bool visibleloading = false;
	bool firstLoadDone = false;

	// level switching
	float changedelay = 0;
	int oldenvironment = 0;

	// level clock
	float loadtime = 0;
	float leveltime = 0;
	float wonleveltime = 0;

	// freeze
	bool freeze = false;
	bool winfreeze = false;

	// tutorial gating
	bool cananger = false;

	// display options
	bool texttoggle = false;
	bool cellophane = false;
	bool foliage = false;
	bool showpoints = false;
	bool showdamagebar = false;
	bool damageeffects = false;
	bool immediate = false;

	// motion blur
	bool alwaysblur = false;
	bool velocityblur = false;
	float motionbluramount = 0;
	float targetblurness = 0;

	// precipitation pacing
	float precipdelay = 0;

	// screen flash
	float flashr = 0;
	float flashg = 0;
	float flashb = 0;
	int flashdelay = 0;

	// stereo
	bool stereoreverse = false;

	// session control
	int tryquit = 0;
	int endgame = 0;
};