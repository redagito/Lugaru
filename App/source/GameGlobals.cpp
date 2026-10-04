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

	SkyBox* skybox = NULL;

	Model hawk;
	Texture hawktexture;
	Vector3 hawkcoords;
	Vector3 realhawkcoords;

	Model eye;
	Model cornea;
	Model iris;

	Vector3 mapcenter;

	Text* text = NULL;
	Text* textmono = NULL;

	Vector3 pathpoint[30];
	int numpathpoints = 0;
	int numpathpointconnect[30] = {};
	int pathpointconnect[30][30] = {};

	std::string consoletext[15] = {};
}