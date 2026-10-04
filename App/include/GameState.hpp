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
	bool canattack = false;
	bool reversaltrain = false;

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
	float stereoseparation = 0.05f;

	// window and input
	bool fullscreen = false;
	bool ismotionblur = false;
	float usermousesensitivity = 0;

	// keybinds, persisted by User/Settings under the same names
	unsigned short crouchkey = 0;
	unsigned short jumpkey = 0;
	unsigned short forwardkey = 0;
	unsigned short backkey = 0;
	unsigned short leftkey = 0;
	unsigned short rightkey = 0;
	unsigned short drawkey = 0;
	unsigned short throwkey = 0;
	unsigned short attackkey = 0;

	// audio
	float volume = 0;
	bool musictoggle = false;
	bool ambientsound = false;

	// texture budget: the square edge length of the screentexture copy, chosen
	// by the detail setting in Game::LoadStuff
	int kTextureSize = 0;

	// skybox tint, and the light it contributes
	float skyboxr = 0;
	float skyboxg = 0;
	float skyboxb = 0;
	float skyboxlightr = 0;
	float skyboxlightg = 0;
	float skyboxlightb = 0;

	// mouse look, accumulated between SDL events and consumed once a frame
	float deltah = 0;
	float deltav = 0;

	// world map
	float mapradius = 0;
	int maptype = 0;

	// editor camera
	float editoryaw = 0;
	float editorpitch = 0;

	// challenge progression
	int numchallengelevels = 0;

	// session start
	bool gamestarted = false;

	// camera wobble from blood loss
	float woozy = 0;

	// smoke texture animation offset
	float smoketex = 0;

	// squared distance beyond which objects are culled
	float playerdist = 0;

	// session control
	int tryquit = 0;
	int endgame = 0;
};