#pragma once

#include "Environment/Lights.hpp"
#include "Graphic/Stereo.hpp"
#include "Math/Frustum.hpp"
#include "Math/Vector3.hpp"

// GameState replaces globals previously declared in App/include/Globals.h and
// App/include/GameGlobals.h. It is expected to be constructed once and passed
// by reference; it owns no storage outside itself, so two instances never
// share values.
//
// Scalars and plain-data arrays. Rendering resources (Texture, Model)
// deliberately stay out: they need a GL context, so putting them here would
// make this struct untestable outside the renderer and non-trivially
// destructible.

struct GameState
{
	// editor
	int editoractive = 0;
	int editorpathtype = 0;
	float editorsize = 0;
	bool editorenabled = false;
	int editortype = 0;

	// which path point the editor is currently working on; -1 means none
	int pathpointselected = 0;

	// the editor's pathfinding graph: the path points, and which of them link to
	// which. numpathpointconnect[i] counts a row of pathpointconnect, and every
	// entry in that row is an index back into pathpoint, so these three index
	// each other and have to move together. They index pathpointselected too,
	// which is what the editor keys its link and delete commands off.
	int numpathpoints = 0;
	Vector3 pathpoint[30] = {};
	int numpathpointconnect[30] = {};
	int pathpointconnect[30][30] = {};

	// hawk
	float hawkyaw = 0;
	float hawkcalldelay = 0;

	// console
	float consoleblinkdelay = 0;
	bool consoleblink = false;
	unsigned consoleselected = 0;
	unsigned short consolekey = 0;
	bool console = false;

	// screen limits
	float maxscreenwidth = 3000;
	float maxscreenheight = 3000;
	float minscreenwidth = 640;
	float minscreenheight = 480;

	// how far the camera can see, and the fraction of that distance at which
	// terrain and sprites start fading out
	float viewdistance = 0;
	float fadestart = 0;

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

	// the time scale the current tick is simulated with, after gamespeed, slow
	// motion and difficulty have been folded into it. realmultiplier above is
	// the same interval before those are applied.
	float multiplier = 0;
	float hostiletime = 0;

	// campaign choice
	int whichchoice = 0;

	// time scale
	float gamespeed = 0;
	float oldgamespeed = 0;
	float slomodelay = 0;
	bool autoslomo = false;

	// whether the whole simulation is currently running slowed down
	int slomo = 0;

	// level loading
	int loading = 0;
	bool stillloading = false;
	bool visibleloading = false;
	bool firstLoadDone = false;

	// whether the next level loads without showing the loading screen
	bool stealthloading = false;

	// level switching
	float changedelay = 0;
	int oldenvironment = 0;

	// which of the snowy, grassy and desert themes the level uses
	int environment = 0;

	// which level to switch to once the current one finishes
	int targetlevel = 0;

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
	bool decalstoggle = false;

	// whether textures are filtered with trilinear mipmapping
	bool trilinear = false;

	// how much blood and blood decals are drawn: 0 off, 1 low detail, 2 high
	int bloodtoggle = 0;

	// devtools: console, level editor and debug info
	bool devtools = false;

	// the LOD bias actually applied to textures, easing towards targetblurness
	float blurness = 0;

	// wind animation phase
	float windvar = 0;

	// world gravity, and the scale the terrain texture is tiled at
	float gravity = 0;
	float texscale = 0;

	// screen darkening, driven by blood loss and damage
	float blackout = 0;

	// whether the level pits the player against hostile characters
	int hostile = 0;

	// whether the skybox is drawn textured rather than as a flat colour
	bool skyboxtexture = false;

	// texture detail the blood and decal code addresses the skin texture with
	float realtexdetail = 0;

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
	float flashamount = 0;

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

	// which keybind row the controls menu is waiting for a key on; -1 means none
	int keyselect = 0;

	// audio
	float volume = 0;
	bool musictoggle = false;
	bool ambientsound = false;

	// which music stream is playing
	int musictype = 0;

	// texture budget: the square edge length of the screentexture copy, chosen
	// by the detail setting in Game::LoadStuff
	int kTextureSize = 0;

	// resolution and detail picked in the options menu, applied on restart
	int newdetail = 0;
	int newscreenwidth = 0;
	int newscreenheight = 0;

	// the resolution and detail those choices were applied to, and the skin
	// texture resolution that detail asks the blood and decal code for
	int detail = 0;
	float screenwidth = 0;
	float screenheight = 0;
	float texdetail = 0;

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

	// camera orientation, which the player's own yaw and pitch are copied from
	// and written back to
	float yaw = 0;
	float pitch = 0;

	// where the camera is, and the direction it faces. Together these are what
	// every culled, faded or wind-blown thing in the world is measured against,
	// so the two move as one: a facing with no position behind it describes
	// nothing.
	Vector3 viewer = {};
	Vector3 viewerfacing = {};

	// the wind the loose sprites are blown by
	Vector3 windvector = {};

	// the one distant light every sprite, object and terrain patch is shaded by.
	// SetUpLight uploads it to a numbered GL light slot; Game::SetUpLighting
	// recolours it from the skybox tint on every level load.
	Light light = {};

	// the six planes of the view volume, rebuilt from the projection and
	// modelview matrices once per frame and read by everything culled or faded.
	Frustum frustum = {};

	// where the hawk perches, and where that perch ends up once the hawk has been
	// swung round its own axis. GameTick derives the second from the first plus
	// hawkyaw every tick, and the draw code tests only the second, so a hawk
	// whose perch moved without the derived position following would be culled
	// by a camera that can see it.
	Vector3 hawkcoords = {};
	Vector3 realhawkcoords = {};

	// the middle of the world map: what the boundary ring is drawn around, and
	// what the map is centred on when it is written out
	Vector3 mapcenter = {};

	// the stereo mode the renderer is using, and the one the options menu is
	// building up for the next restart. The menu copies the first into the second
	// on entry, walks the second forward one mode at a time, and only commits it
	// back to the first once the renderer has agreed to initialise it.
	StereoMode stereomode = stereoNone;
	StereoMode newstereomode = stereoNone;

	// challenge progression
	int numchallengelevels = 0;

	// difficulty the active account plays at: 0 easy, 1 medium, 2 insane
	int difficulty = 0;

	// session start
	bool gamestarted = false;

	// camera wobble from blood loss
	float woozy = 0;

	// smoke texture animation offset
	float smoketex = 0;

	// squared distance beyond which objects are culled
	float playerdist = 0;

	// skeletal animation: which joint each bone spans. Row i of the two tables
	// together describe one bone, the joint it starts from and the joint it ends
	// at, so the tables are only meaningful as a pair and move as one. They are
	// built once per session by Game::InitGame and read for the rest of it, and
	// Skeleton::DoConstraints walks them to a fixed 26 rows, so the extent is
	// part of the contract rather than a stored count.
	int whichjointstartarray[26] = {};
	int whichjointendarray[26] = {};

	// session control
	int tryquit = 0;
	int endgame = 0;

	// which menu is showing; Menu::Load lists what each value means
	int mainmenu = 0;

	// which item of that menu is highlighted; -1 means none
	int selected = 0;

	// how far blood loss shakes the camera
	float camerashake = 0;

	// whether a game is in progress rather than sitting in the menus
	bool gameon = false;

	// whether the camera is detached from the player and free-flying
	bool cameramode = false;

	// whether text input has been requested and is still collecting characters
	bool waiting = false;
};