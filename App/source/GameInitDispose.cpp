/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Game.hpp"
#include "GameGlobals.h"

#include "Animation/Animation.hpp"
#include "Audio/openal_wrapper.hpp"
#include "CommandLine.hpp"
#include "GameState.hpp"
#include "Graphic/Texture.hpp"
#include "LoadingClock.hpp"
#include "Menu/Menu.hpp"
#include "Utils/Folders.hpp"

#include "Globals.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <string>
#include <GL/glext.h>
#include <Graphic/Sprite.hpp>
#include <Objects/Person.hpp>
#include <Objects/Weapons.hpp>
#include <Audio/Sounds.hpp>
#include <Math/Vector3.hpp>
#include <Platform/Platform.hpp>
#include <Utils/ImageIO.hpp>
#include <User/Account.hpp>
#include <Animation/Animation.def>
#include <Animation/Joint.hpp>
#include <Environment/Skybox.hpp>
#include <Graphic/Text.hpp>
#include <Objects/PersonType.hpp>

extern float accountcampaignhighscore[10];
extern float accountcampaignfasttime[10];
extern float accountcampaignscore[10];
extern float accountcampaigntime[10];

extern int accountcampaignchoicesmade[10];
extern int accountcampaignchoices[10][5000];

void Dispose(GameState& gamestate)
{

	if (gamestate.endgame == 2) {
		Account::active().endGame();
		gamestate.endgame = 0;
	}

	Account::saveFile(Folders::getUserSavePath());

	//textures.clear();

	OPENAL_StopSound(OPENAL_ALL);

	for (int i = 0; i < sounds_count; ++i) {
		OPENAL_Sample_Free(samp[i]);
	}

	OPENAL_Close();
}

void Game::newGame()
{
	text = new Text();
	textmono = new Text();
	skybox = new SkyBox();
}

void Game::deleteGame(GameState& gamestate)
{
	delete skybox;
	delete text;
	delete textmono;

	glDeleteTextures(1, &screentexture);
	glDeleteTextures(1, &screentexture2);

	Dispose(gamestate);
}

void LoadSave(const std::string& fileName, GLubyte* array, GameState& gamestate)
{

	//Load Image
	float temptexdetail = texdetail;
	texdetail = 1;

	//Load Image
	ImageRec texture;
	if (!load_image(Folders::getResourcePath(fileName).c_str(), texture, [&]() {Game::LoadingScreen(gamestate); })) {
		texdetail = temptexdetail;
		return;
	}
	texdetail = temptexdetail;

	int bytesPerPixel = texture.bpp / 8;

	int tempnum = 0;
	for (int i = 0; i < (int)(texture.sizeY * texture.sizeX * bytesPerPixel); i++) {
		if ((i + 1) % 4 || bytesPerPixel == 3) {
			array[tempnum] = texture.data[i];
			tempnum++;
		}
	}
}

//***************> ResizeGLScene() <******/
GLvoid Game::ReSizeGLScene(float fov, float pnear)
{
	if (screenheight == 0) {
		screenheight = 1;
	}

	glViewport(0, 0, screenwidth, screenheight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(fov, (GLfloat)screenwidth / (GLfloat)screenheight, pnear, viewdistance);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Game::LoadingScreen(GameState& gamestate)
{
	if (!gamestate.visibleloading) {
		return;
	}

	static float loadprogress;
	// The overlay's own wall-clock state. It is deliberately not the game-wide
	// frame delta: this function is a progress callback, and the ramp and the
	// flash must not depend on how the game loop happens to be paced.
	static LoadingClock clock;
	static AbsoluteTime frametime = { 0, 0 };
	AbsoluteTime currTime = UpTime();
	double deltaTime = (float)AbsoluteDeltaToDuration(currTime, frametime);

	if (0 > deltaTime) { // if negative microseconds
		deltaTime /= -1000000.0;
	}
	else { // else milliseconds
		deltaTime /= 1000.0;
	}

	const float step = (float)deltaTime;

	// The very first call only takes a baseline. Measuring against a
	// zero-initialised frametime would count the whole process uptime as this
	// load's first frame.
	if (!clock.primed()) {
		clock.advance(0.0f);
		frametime = currTime;
		return;
	}

	// The rest of the game zeroes gamestate.loadtime between levels. That is the
	// signal that this load wants a fresh pulse, so drop the banked time and
	// take a new baseline rather than counting the dead time since the last one.
	if (gamestate.loadtime == 0.0f && clock.ramp() > 0.0f) {
		clock.reset();
		frametime = currTime;
		return;
	}

	if (step <= .05f) return;

	frametime = currTime; // reset for next time interval

	// Only redraws bank time, so frametime above advances by exactly the gap this
	// redraw covers: a ramp takes a fixed amount of wall-clock time no matter how
	// often the throttle skips a frame.
	const float elapsed = clock.advance(step);

	// Devtools-only: trace what drives the loading overlay. A correct trace now
	// shows loadprogress climbing at a steady wall-clock rate and flashamount
	// bleeding off instead of sitting pinned at 1.
	if (gamestate.devtools) {
		static int tracecount = 0;
		if (tracecount < 400 && (tracecount % 10) == 0) {
			fprintf(stderr, "[loading] n=%d elapsed=%.4f loadprogress=%.2f flashamount=%.3f\n",
					tracecount, elapsed, clock.ramp(), gamestate.flashamount);
		}
		tracecount++;
	}

	glLoadIdentity();
	//Clear to black
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// The ramp is paced by real seconds, so a full 0 -> 100 always takes
	// LoadingClock::rampSeconds of wall-clock time however often the throttle
	// skips a frame. gamestate.loadtime is kept in sync because the rest of the
	// game zeroes it between levels.
	loadprogress = clock.ramp();
	gamestate.loadtime = loadprogress;

	//Background

	glEnable(GL_TEXTURE_2D);
	loadscreentexture.bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDepthMask(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, screenwidth, 0, screenheight, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(screenwidth / 2, screenheight / 2, 0);
	glScalef((float)screenwidth / 2, (float)screenheight / 2, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glColor4f(loadprogress / 100, loadprogress / 100, loadprogress / 100, 1);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(.1 - loadprogress / 100, 0 + loadprogress / 100 + .3);
	glVertex3f(-1, -1, 0.0f);
	glTexCoord2f(.1 - loadprogress / 100, 0 + loadprogress / 100 + .3);
	glVertex3f(1, -1, 0.0f);
	glTexCoord2f(.1 - loadprogress / 100, 1 + loadprogress / 100 + .3);
	glVertex3f(1, 1, 0.0f);
	glTexCoord2f(.1 - loadprogress / 100, 1 + loadprogress / 100 + .3);
	glVertex3f(-1, 1, 0.0f);
	glEnd();
	glPopMatrix();
	glEnable(GL_BLEND);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(.4 + loadprogress / 100, 0 + loadprogress / 100);
	glVertex3f(-1, -1, 0.0f);
	glTexCoord2f(.4 + loadprogress / 100, 0 + loadprogress / 100);
	glVertex3f(1, -1, 0.0f);
	glTexCoord2f(.4 + loadprogress / 100, 1 + loadprogress / 100);
	glVertex3f(1, 1, 0.0f);
	glTexCoord2f(.4 + loadprogress / 100, 1 + loadprogress / 100);
	glVertex3f(-1, 1, 0.0f);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glDisable(GL_BLEND);
	glDepthMask(1);

	glEnable(GL_TEXTURE_2D);
	loadscreentexture.bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDepthMask(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, screenwidth, 0, screenheight, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(screenwidth / 2, screenheight / 2, 0);
	glScalef((float)screenwidth / 2 * (1.5 - (loadprogress) / 200), (float)screenheight / 2 * (1.5 - (loadprogress) / 200), 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
	glColor4f(loadprogress / 100, loadprogress / 100, loadprogress / 100, 1);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0 + .5, 0 + .5);
	glVertex3f(-1, -1, 0.0f);
	glTexCoord2f(1 + .5, 0 + .5);
	glVertex3f(1, -1, 0.0f);
	glTexCoord2f(1 + .5, 1 + .5);
	glVertex3f(1, 1, 0.0f);
	glTexCoord2f(0 + .5, 1 + .5);
	glVertex3f(-1, 1, 0.0f);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glDisable(GL_BLEND);
	glDepthMask(1);

	glEnable(GL_TEXTURE_2D);
	loadscreentexture.bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDepthMask(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, screenwidth, 0, screenheight, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(screenwidth / 2, screenheight / 2, 0);
	glScalef((float)screenwidth / 2 * (100 + loadprogress) / 100, (float)screenheight / 2 * (100 + loadprogress) / 100, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
	glColor4f(loadprogress / 100, loadprogress / 100, loadprogress / 100, .4);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0 + .2, 0 + .8);
	glVertex3f(-1, -1, 0.0f);
	glTexCoord2f(1 + .2, 0 + .8);
	glVertex3f(1, -1, 0.0f);
	glTexCoord2f(1 + .2, 1 + .8);
	glVertex3f(1, 1, 0.0f);
	glTexCoord2f(0 + .2, 1 + .8);
	glVertex3f(-1, 1, 0.0f);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glDisable(GL_BLEND);
	glDepthMask(1);

	//Text

	if (gamestate.flashamount > 0) {
		if (gamestate.flashamount > 1) {
			gamestate.flashamount = 1;
		}
		if (gamestate.flashdelay <= 0) {
			gamestate.flashamount = LoadingClock::decayFlash(gamestate.flashamount, elapsed);
		}
		gamestate.flashdelay--;
		if (gamestate.flashamount < 0) {
			gamestate.flashamount = 0;
		}
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glDepthMask(0);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, screenwidth, 0, screenheight, -100, 100);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glScalef(screenwidth, screenheight, 1);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glColor4f(gamestate.flashr, gamestate.flashg, gamestate.flashb, gamestate.flashamount);
		glBegin(GL_QUADS);
		glVertex3f(0, 0, 0.0f);
		glVertex3f(256, 0, 0.0f);
		glVertex3f(256, 256, 0.0f);
		glVertex3f(0, 256, 0.0f);
		glEnd();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDepthMask(1);
	}

	swap_gl_buffers();
}

void FadeLoadingScreen(float howmuch)
{
	static float loadprogress;

	glLoadIdentity();
	//Clear to black
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	loadprogress = howmuch;

	//Background

	glDisable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDepthMask(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, screenwidth, 0, screenheight, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(screenwidth / 2, screenheight / 2, 0);
	glScalef((float)screenwidth / 2, (float)screenheight / 2, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glColor4f(loadprogress / 100, 0, 0, 1);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-1, -1, 0.0f);
	glTexCoord2f(1, 0);
	glVertex3f(1, -1, 0.0f);
	glTexCoord2f(1, 1);
	glVertex3f(1, 1, 0.0f);
	glTexCoord2f(0, 1);
	glVertex3f(-1, 1, 0.0f);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glDisable(GL_BLEND);
	glDepthMask(1);
	//Text
	swap_gl_buffers();
}

void Game::InitGame(GameState& gamestate)
{
	gamestate.numchallengelevels = 14;

	Account::loadFile(Folders::getUserSavePath());

	whichjointstartarray[0] = righthip;
	whichjointendarray[0] = rightfoot;

	whichjointstartarray[1] = righthip;
	whichjointendarray[1] = rightankle;

	whichjointstartarray[2] = righthip;
	whichjointendarray[2] = rightknee;

	whichjointstartarray[3] = rightknee;
	whichjointendarray[3] = rightankle;

	whichjointstartarray[4] = rightankle;
	whichjointendarray[4] = rightfoot;

	whichjointstartarray[5] = lefthip;
	whichjointendarray[5] = leftfoot;

	whichjointstartarray[6] = lefthip;
	whichjointendarray[6] = leftankle;

	whichjointstartarray[7] = lefthip;
	whichjointendarray[7] = leftknee;

	whichjointstartarray[8] = leftknee;
	whichjointendarray[8] = leftankle;

	whichjointstartarray[9] = leftankle;
	whichjointendarray[9] = leftfoot;

	whichjointstartarray[10] = abdomen;
	whichjointendarray[10] = rightshoulder;

	whichjointstartarray[11] = abdomen;
	whichjointendarray[11] = rightelbow;

	whichjointstartarray[12] = abdomen;
	whichjointendarray[12] = rightwrist;

	whichjointstartarray[13] = abdomen;
	whichjointendarray[13] = righthand;

	whichjointstartarray[14] = rightshoulder;
	whichjointendarray[14] = rightelbow;

	whichjointstartarray[15] = rightelbow;
	whichjointendarray[15] = rightwrist;

	whichjointstartarray[16] = rightwrist;
	whichjointendarray[16] = righthand;

	whichjointstartarray[17] = abdomen;
	whichjointendarray[17] = leftshoulder;

	whichjointstartarray[18] = abdomen;
	whichjointendarray[18] = leftelbow;

	whichjointstartarray[19] = abdomen;
	whichjointendarray[19] = leftwrist;

	whichjointstartarray[20] = abdomen;
	whichjointendarray[20] = lefthand;

	whichjointstartarray[21] = leftshoulder;
	whichjointendarray[21] = leftelbow;

	whichjointstartarray[22] = leftelbow;
	whichjointendarray[22] = leftwrist;

	whichjointstartarray[23] = leftwrist;
	whichjointendarray[23] = lefthand;

	whichjointstartarray[24] = abdomen;
	whichjointendarray[24] = neck;

	whichjointstartarray[25] = neck;
	whichjointendarray[25] = head;

	FadeLoadingScreen(0);

	gamestate.stillloading = 1;

	int temptexdetail = texdetail;
	texdetail = 1;
	text->LoadFontTexture("Textures/Font.png", trilinear, [&]() { Game::LoadingScreen(gamestate); });
	text->BuildFont();
	textmono->LoadFontTexture("Textures/FontMono.png", trilinear, [&]() { Game::LoadingScreen(gamestate); });
	textmono->BuildFont();
	texdetail = temptexdetail;

	FadeLoadingScreen(10);

	if (detail == 2) {
		texdetail = 1;
	}
	if (detail == 1) {
		texdetail = 2;
	}
	if (detail == 0) {
		texdetail = 4;
	}

	// 
	OPENAL_Init(44100, 32, 0, commandLineOptions[OPENALINFO]);

	OPENAL_SetSFXMasterVolume((int)(gamestate.volume * 255));
	loadAllSounds();

	if (gamestate.musictoggle) {
		emit_stream_np(stream_menutheme);
	}

	cursortexture.load("Textures/Cursor.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });

	Mapcircletexture.load("Textures/MapCircle.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });
	Mapboxtexture.load("Textures/MapBox.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });
	Maparrowtexture.load("Textures/MapArrow.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });

	temptexdetail = texdetail;
	if (texdetail > 2) {
		texdetail = 2;
	}
	Mainmenuitems[0].load("Textures/Lugaru.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });
	Mainmenuitems[1].load("Textures/NewGame.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });
	Mainmenuitems[2].load("Textures/Options.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });
	Mainmenuitems[3].load("Textures/Quit.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });
	Mainmenuitems[4].load("Textures/Eyelid.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });
	Mainmenuitems[5].load("Textures/Resume.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });
	Mainmenuitems[6].load("Textures/EndGame.png", 0, trilinear, [&]() {Game::LoadingScreen(gamestate); });

	texdetail = temptexdetail;

	FadeLoadingScreen(95);

	gamestate.gameon = 0;
	mainmenu = 1;

	gamestate.stillloading = 0;
	gamestate.firstLoadDone = false;

	gamestate.newdetail = detail;
	gamestate.newscreenwidth = screenwidth;
	gamestate.newscreenheight = screenheight;

	Menu::Load(gamestate);

	Animation::loadAll([&]() {Game::LoadingScreen(gamestate); });

	PersonType::Load();

	Person::players.emplace_back(new Person(gamestate));
}

void Game::LoadScreenTexture(GameState& gamestate)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (!Game::screentexture) {
		glGenTextures(1, &Game::screentexture);
	}
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, Game::screentexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, gamestate.kTextureSize, gamestate.kTextureSize, 0);
}

//TODO: move LoadStuff() closer to GameTick.cpp to get rid of various vars shared in Game.hpp
/* Loads models and textures which only needs to be loaded once */
void Game::LoadStuff(GameState& gamestate)
{
	float temptexdetail;
	float viewdistdetail;
	float megascale = 1;

	gamestate.loadtime = 0;

	gamestate.stillloading = 1;

	gamestate.visibleloading = false; //don't use loadscreentexture yet
	loadscreentexture.load("Textures/Fire.jpg", 1, trilinear, [&]() {Game::LoadingScreen(gamestate); });
	gamestate.visibleloading = true;

	temptexdetail = texdetail;
	texdetail = 1;
	text->LoadFontTexture("Textures/Font.png", trilinear, [&]() { Game::LoadingScreen(gamestate); });
	text->BuildFont();
	textmono->LoadFontTexture("Textures/FontMono.png", trilinear, [&]() { Game::LoadingScreen(gamestate); });
	textmono->BuildFont();
	texdetail = temptexdetail;

	viewdistdetail = 2;
	viewdistance = 50 * megascale * viewdistdetail;

	if (detail == 2) {
		texdetail = 1;
		gamestate.kTextureSize = 1024;
	}
	else if (detail == 1) {
		texdetail = 2;
		gamestate.kTextureSize = 512;
	}
	else {
		texdetail = 4;
		gamestate.kTextureSize = 256;
	}

	gamestate.realtexdetail = texdetail;

	Weapon::Load(trilinear, [&]() { Game::LoadingScreen(gamestate); });

	terrain.shadowtexture.load("Textures/Shadow.png", 0, trilinear, [&]() {LoadingScreen(gamestate); });
	terrain.bloodtexture.load("Textures/Blood.png", 0, trilinear, [&]() {LoadingScreen(gamestate); });
	terrain.breaktexture.load("Textures/Break.png", 0, trilinear, [&]() {LoadingScreen(gamestate); });
	terrain.bloodtexture2.load("Textures/Blood.png", 0, trilinear, [&]() {LoadingScreen(gamestate); });

	terrain.footprinttexture.load("Textures/Footprint.png", 0, trilinear, [&]() {LoadingScreen(gamestate); });
	terrain.bodyprinttexture.load("Textures/Bodyprint.png", 0, trilinear, [&]() {LoadingScreen(gamestate); });
	hawktexture.load("Textures/Hawk.png", 0, trilinear, [&]() { LoadingScreen(gamestate); });

	Sprite::cloudtexture.load("Textures/Cloud.png", 1, trilinear, [&]() {LoadingScreen(gamestate); });
	Sprite::cloudimpacttexture.load("Textures/CloudImpact.png", 1, trilinear, [&]() { LoadingScreen(gamestate); });
	Sprite::bloodtexture.load("Textures/BloodParticle.png", 1, trilinear, [&]() { LoadingScreen(gamestate); });
	Sprite::snowflaketexture.load("Textures/SnowFlake.png", 1, trilinear, [&]() {LoadingScreen(gamestate); });
	Sprite::flametexture.load("Textures/Flame.png", 1, trilinear, [&]() {LoadingScreen(gamestate); });
	Sprite::bloodflametexture.load("Textures/BloodFlame.png", 1, trilinear, [&]() {LoadingScreen(gamestate); });
	Sprite::smoketexture.load("Textures/Smoke.png", 1, trilinear, [&]() {LoadingScreen(gamestate); });
	Sprite::shinetexture.load("Textures/Shine.png", 1, trilinear, [&]() {LoadingScreen(gamestate); });
	Sprite::splintertexture.load("Textures/Splinter.png", 1, trilinear, [&]() {LoadingScreen(gamestate); });
	Sprite::leaftexture.load("Textures/Leaf.png", 1, trilinear, [&]() {LoadingScreen(gamestate); });
	Sprite::toothtexture.load("Textures/Tooth.png", 1, trilinear, [&]() {LoadingScreen(gamestate); });

	yaw = 0;
	pitch = 0;
	ReSizeGLScene(90, .01);

	viewer = 0;

	//Set up distant light
	light.color[0] = .95;
	light.color[1] = .95;
	light.color[2] = 1;
	light.ambient[0] = .2;
	light.ambient[1] = .2;
	light.ambient[2] = .24;
	light.location.x = 1;
	light.location.y = 1;
	light.location.z = -.2;
	Normalise(&light.location);

	LoadingScreen(gamestate);

	SetUpLighting(gamestate);

	fadestart = .6;
	gamestate.gravity = -10;

	gamestate.texscale = .2 / megascale / viewdistdetail;
	terrain.scale = 3 * megascale * viewdistdetail;

	viewer.x = terrain.size / 2 * terrain.scale;
	viewer.z = terrain.size / 2 * terrain.scale;

	hawk.load("Models/Hawk.solid", [&]() { LoadingScreen(gamestate); });
	hawk.Scale(.03, .03, .03);
	hawk.Rotate(90, 1, 1);
	hawk.CalculateNormals(0, [&]() { LoadingScreen(gamestate); });
	hawk.ScaleNormals(-1, -1, -1);
	hawkcoords.x = terrain.size / 2 * terrain.scale - 5 - 7;
	hawkcoords.z = terrain.size / 2 * terrain.scale - 5 - 7;
	hawkcoords.y = terrain.getHeight(hawkcoords.x, hawkcoords.z) + 25;

	eye.load("Models/Eye.solid", [&]() { Game::LoadingScreen(gamestate); });
	eye.Scale(.03, .03, .03);
	eye.CalculateNormals(0, [&]() { LoadingScreen(gamestate); });

	cornea.load("Models/Cornea.solid", [&]() { Game::LoadingScreen(gamestate); });
	cornea.Scale(.03, .03, .03);
	cornea.CalculateNormals(0, [&]() { Game::LoadingScreen(gamestate); });

	iris.load("Models/Iris.solid", [&]() { Game::LoadingScreen(gamestate); });
	iris.Scale(.03, .03, .03);
	iris.CalculateNormals(0, [&]() { LoadingScreen(gamestate); });

	LoadSave("Textures/WolfBloodFur.png", &PersonType::types[wolftype].bloodText[0], gamestate);
	LoadSave("Textures/BloodFur.png", &PersonType::types[rabbittype].bloodText[0], gamestate);

	gamestate.oldenvironment = -4;

	gamestate.gameon = 1;
	mainmenu = 0;

	//Fix knife stab, too lazy to do it manually
	Vector3 moveamount;
	moveamount = 0;
	moveamount.z = 2;
	// FIXME - Why this uses skeleton.joints.size() and not Animation::numjoints? (are they equal?)
	// It seems skeleton.joints.size() is 0 at this point, so this is useless.
	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		for (unsigned j = 0; j < Animation::animations[knifesneakattackanim].frames.size(); j++) {
			Animation::animations[knifesneakattackanim].frames[j].joints[i].position += moveamount;
		}
	}

	LoadingScreen(gamestate);

	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		for (unsigned j = 0; j < Animation::animations[knifesneakattackedanim].frames.size(); j++) {
			Animation::animations[knifesneakattackedanim].frames[j].joints[i].position += moveamount;
		}
	}

	LoadingScreen(gamestate);

	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		Animation::animations[dead1anim].frames[1].joints[i].position = Animation::animations[dead1anim].frames[0].joints[i].position;
		Animation::animations[dead2anim].frames[1].joints[i].position = Animation::animations[dead2anim].frames[0].joints[i].position;
		Animation::animations[dead3anim].frames[1].joints[i].position = Animation::animations[dead3anim].frames[0].joints[i].position;
		Animation::animations[dead4anim].frames[1].joints[i].position = Animation::animations[dead4anim].frames[0].joints[i].position;
	}
	Animation::animations[dead1anim].frames[0].speed = 0.001;
	Animation::animations[dead2anim].frames[0].speed = 0.001;
	Animation::animations[dead3anim].frames[0].speed = 0.001;
	Animation::animations[dead4anim].frames[0].speed = 0.001;

	Animation::animations[dead1anim].frames[1].speed = 0.001;
	Animation::animations[dead2anim].frames[1].speed = 0.001;
	Animation::animations[dead3anim].frames[1].speed = 0.001;
	Animation::animations[dead4anim].frames[1].speed = 0.001;

	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		for (unsigned j = 0; j < Animation::animations[swordsneakattackanim].frames.size(); j++) {
			Animation::animations[swordsneakattackanim].frames[j].joints[i].position += moveamount;
		}
	}
	LoadingScreen(gamestate);
	for (unsigned j = 0; j < Animation::animations[swordsneakattackanim].frames.size(); j++) {
		Animation::animations[swordsneakattackanim].frames[j].weapontarget += moveamount;
	}

	LoadingScreen(gamestate);

	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		for (unsigned j = 0; j < Animation::animations[swordsneakattackedanim].frames.size(); j++) {
			Animation::animations[swordsneakattackedanim].frames[j].joints[i].position += moveamount;
		}
	}

	LoadingScreen(gamestate);

	if (!screentexture) {
		LoadScreenTexture(gamestate);
	}

	if (gamestate.targetlevel != 7) {
		emit_sound_at(fireendsound);
	}

	gamestate.stillloading = 0;
	gamestate.loading = 0;
	gamestate.changedelay = 1;

	gamestate.visibleloading = false;
	gamestate.firstLoadDone = true;
}
