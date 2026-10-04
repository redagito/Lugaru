#pragma once

#include <string>

#include "Environment/Skybox.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/Text.hpp"
#include "Graphic/Models.hpp"

namespace Game
{
	extern Texture terraintexture;
	extern Texture terraintexture2;
	extern Texture loadscreentexture;
	extern Texture Mapcircletexture;
	extern Texture Maparrowtexture;
	extern Texture Mapboxtexture;
	extern Texture cursortexture;
	extern GLuint screentexture;
	extern GLuint screentexture2;
	extern Texture Mainmenuitems[10];

	extern int selected;
	extern int keyselect;

	extern int newdetail;
	extern int newscreenwidth;
	extern int newscreenheight;

	extern bool gameon;
	extern float yaw, pitch;
	extern SkyBox* skybox;
	extern bool cameramode;

	extern Model hawk;
	extern Texture hawktexture;
	extern Vector3 hawkcoords;
	extern Vector3 realhawkcoords;

	extern Model eye;
	extern Model cornea;
	extern Model iris;

	extern bool stealthloading;

	extern int musictype;

	extern Vector3 mapcenter;

	extern Text* text;
	extern Text* textmono;

	extern bool editorenabled;
	extern int editortype;

	extern Vector3 pathpoint[30];
	extern int numpathpoints;
	extern int numpathpointconnect[30];
	extern int pathpointconnect[30][30];
	extern int pathpointselected;

	extern bool console;
	extern std::string consoletext[15];

	extern int targetlevel;

	extern bool waiting;
}