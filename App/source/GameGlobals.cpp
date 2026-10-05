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

	Model hawk;
	Texture hawktexture;

	Model eye;
	Model cornea;
	Model iris;

	std::string consoletext[15] = {};
}