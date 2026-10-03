#include "Game.hpp"

namespace Game
{
	Texture terraintexture;
	Texture terraintexture2;
	Texture loadscreentexture;
	Texture Mapcircletexture;
	Texture Maparrowtexture;
	Texture Mapboxtexture;
	Texture cursortexture;
	GLuint screentexture = 0;
	GLuint screentexture2 = 0;
	Texture Mainmenuitems[10];

	int selected = 0;
	int keyselect = 0;

	int newdetail = 0;
	int newscreenwidth = 0;
	int newscreenheight = 0;

	bool gameon = 0;
	float deltah = 0;
	float deltav = 0;
	int mousecoordh = 0;
	int mousecoordv = 0;
	int oldmousecoordh = 0;
	float yaw = 0;
	float pitch = 0;
	SkyBox* skybox = NULL;
	bool cameramode = 0;
	bool firstLoadDone = false;

	float leveltime = 0;
	float wonleveltime = 0;
	float loadtime = 0;

	Model hawk;
	Texture hawktexture;
	Vector3 hawkcoords;
	Vector3 realhawkcoords;

	Model eye;
	Model cornea;
	Model iris;

	bool stealthloading = 0;

	int musictype = 0;

	Vector3 mapcenter;
	float mapradius = 0;

	Text* text = NULL;
	Text* textmono = NULL;

	bool editorenabled = 0;
	int editortype = 0;
	float editorsize = 0;
	float editoryaw = 0;
	float editorpitch = 0;

	int tryquit = 0;

	Vector3 pathpoint[30];
	int numpathpoints = 0;
	int numpathpointconnect[30] = {};
	int pathpointconnect[30][30] = {};
	int pathpointselected = 0;

	int endgame = 0;
	int numchallengelevels = 0;

	bool console = false;
	std::string consoletext[15] = {};
	unsigned consoleselected = 0;

	unsigned short crouchkey = 0, jumpkey = 0, forwardkey = 0, backkey = 0, leftkey = 0, rightkey = 0, drawkey = 0, throwkey = 0, attackkey = 0;
	unsigned short consolekey = 0;

	int loading = 0;

	int oldenvironment = 0;
	int targetlevel = 0;
	float changedelay = 0;

	bool waiting = false;
}