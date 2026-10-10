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

#include "Animation/Animation.hpp"
#include "Audio/openal_wrapper.hpp"
#include "CommandLine.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Graphic/Texture.hpp"
#include "KeyCapture.hpp"
#include "LoadingClock.hpp"
#include "Menu/Menu.hpp"
#include "Utils/Folders.hpp"
#include "WindowContext.hpp"

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
#include <Animation/Animation.inc>
#include <Animation/Joint.hpp>
#include <Environment/Skybox.hpp>
#include <Graphic/Text.hpp>
#include <Objects/PersonType.hpp>

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

void Game::deleteGame(GameState& gamestate, GameAssets& assets)
{
	glDeleteTextures(1, &assets.screentexture);
	glDeleteTextures(1, &assets.screentexture2);

	Dispose(gamestate);
}

void LoadSave(const std::string& fileName, GLubyte* array, GameState& gamestate, GameAssets& assets)
{

	//Load Image
	float temptexdetail = gamestate.texdetail;
	gamestate.texdetail = 1;

	//Load Image
	ImageRec texture;
	if (!load_image(Folders::getResourcePath(fileName).c_str(), texture, [&]() {Game::LoadingScreen(gamestate, assets); })) {
		gamestate.texdetail = temptexdetail;
		return;
	}
	gamestate.texdetail = temptexdetail;

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
GLvoid Game::ReSizeGLScene(float fov, float pnear, GameState& gamestate)
{
	if (gamestate.screenheight == 0) {
		gamestate.screenheight = 1;
	}

	glViewport(0, 0, gamestate.screenwidth, gamestate.screenheight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(fov, (GLfloat)gamestate.screenwidth / (GLfloat)gamestate.screenheight, pnear, gamestate.viewdistance);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Game::LoadingScreen(GameState& gamestate, GameAssets& assets)
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
	assets.loadscreentexture.bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDepthMask(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, gamestate.screenwidth, 0, gamestate.screenheight, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(gamestate.screenwidth / 2, gamestate.screenheight / 2, 0);
	glScalef((float)gamestate.screenwidth / 2, (float)gamestate.screenheight / 2, 1);
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
	assets.loadscreentexture.bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDepthMask(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, gamestate.screenwidth, 0, gamestate.screenheight, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(gamestate.screenwidth / 2, gamestate.screenheight / 2, 0);
	glScalef((float)gamestate.screenwidth / 2 * (1.5 - (loadprogress) / 200), (float)gamestate.screenheight / 2 * (1.5 - (loadprogress) / 200), 1);
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
	assets.loadscreentexture.bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDepthMask(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, gamestate.screenwidth, 0, gamestate.screenheight, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(gamestate.screenwidth / 2, gamestate.screenheight / 2, 0);
	glScalef((float)gamestate.screenwidth / 2 * (100 + loadprogress) / 100, (float)gamestate.screenheight / 2 * (100 + loadprogress) / 100, 1);
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
		glOrtho(0, gamestate.screenwidth, 0, gamestate.screenheight, -100, 100);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glScalef(gamestate.screenwidth, gamestate.screenheight, 1);
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

	swap_gl_buffers(WindowContext::mainWindow());
}

void FadeLoadingScreen(float howmuch, GameState& gamestate)
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
	glOrtho(0, gamestate.screenwidth, 0, gamestate.screenheight, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(gamestate.screenwidth / 2, gamestate.screenheight / 2, 0);
	glScalef((float)gamestate.screenwidth / 2, (float)gamestate.screenheight / 2, 1);
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
	swap_gl_buffers(WindowContext::mainWindow());
}

void Game::InitGame(GameState& gamestate, GameAssets& assets, KeyCapture& keycapture)
{
	gamestate.numchallengelevels = 14;

	Account::loadFile(Folders::getUserSavePath());

	gamestate.whichjointstartarray[0] = righthip;
	gamestate.whichjointendarray[0] = rightfoot;

	gamestate.whichjointstartarray[1] = righthip;
	gamestate.whichjointendarray[1] = rightankle;

	gamestate.whichjointstartarray[2] = righthip;
	gamestate.whichjointendarray[2] = rightknee;

	gamestate.whichjointstartarray[3] = rightknee;
	gamestate.whichjointendarray[3] = rightankle;

	gamestate.whichjointstartarray[4] = rightankle;
	gamestate.whichjointendarray[4] = rightfoot;

	gamestate.whichjointstartarray[5] = lefthip;
	gamestate.whichjointendarray[5] = leftfoot;

	gamestate.whichjointstartarray[6] = lefthip;
	gamestate.whichjointendarray[6] = leftankle;

	gamestate.whichjointstartarray[7] = lefthip;
	gamestate.whichjointendarray[7] = leftknee;

	gamestate.whichjointstartarray[8] = leftknee;
	gamestate.whichjointendarray[8] = leftankle;

	gamestate.whichjointstartarray[9] = leftankle;
	gamestate.whichjointendarray[9] = leftfoot;

	gamestate.whichjointstartarray[10] = abdomen;
	gamestate.whichjointendarray[10] = rightshoulder;

	gamestate.whichjointstartarray[11] = abdomen;
	gamestate.whichjointendarray[11] = rightelbow;

	gamestate.whichjointstartarray[12] = abdomen;
	gamestate.whichjointendarray[12] = rightwrist;

	gamestate.whichjointstartarray[13] = abdomen;
	gamestate.whichjointendarray[13] = righthand;

	gamestate.whichjointstartarray[14] = rightshoulder;
	gamestate.whichjointendarray[14] = rightelbow;

	gamestate.whichjointstartarray[15] = rightelbow;
	gamestate.whichjointendarray[15] = rightwrist;

	gamestate.whichjointstartarray[16] = rightwrist;
	gamestate.whichjointendarray[16] = righthand;

	gamestate.whichjointstartarray[17] = abdomen;
	gamestate.whichjointendarray[17] = leftshoulder;

	gamestate.whichjointstartarray[18] = abdomen;
	gamestate.whichjointendarray[18] = leftelbow;

	gamestate.whichjointstartarray[19] = abdomen;
	gamestate.whichjointendarray[19] = leftwrist;

	gamestate.whichjointstartarray[20] = abdomen;
	gamestate.whichjointendarray[20] = lefthand;

	gamestate.whichjointstartarray[21] = leftshoulder;
	gamestate.whichjointendarray[21] = leftelbow;

	gamestate.whichjointstartarray[22] = leftelbow;
	gamestate.whichjointendarray[22] = leftwrist;

	gamestate.whichjointstartarray[23] = leftwrist;
	gamestate.whichjointendarray[23] = lefthand;

	gamestate.whichjointstartarray[24] = abdomen;
	gamestate.whichjointendarray[24] = neck;

	gamestate.whichjointstartarray[25] = neck;
	gamestate.whichjointendarray[25] = head;

	FadeLoadingScreen(0, gamestate);

	gamestate.stillloading = 1;

	int temptexdetail = gamestate.texdetail;
	gamestate.texdetail = 1;
	assets.text.LoadFontTexture("Textures/Font.png", gamestate.trilinear, [&]() { Game::LoadingScreen(gamestate, assets); });
	assets.text.BuildFont();
	assets.textmono.LoadFontTexture("Textures/FontMono.png", gamestate.trilinear, [&]() { Game::LoadingScreen(gamestate, assets); });
	assets.textmono.BuildFont();
	gamestate.texdetail = temptexdetail;

	FadeLoadingScreen(10, gamestate);

	if (gamestate.detail == 2) {
		gamestate.texdetail = 1;
	}
	if (gamestate.detail == 1) {
		gamestate.texdetail = 2;
	}
	if (gamestate.detail == 0) {
		gamestate.texdetail = 4;
	}

	// 
	OPENAL_Init(44100, 32, 0, commandLineOptions[OPENALINFO]);

	OPENAL_SetSFXMasterVolume((int)(gamestate.volume * 255));
	loadAllSounds();

	if (gamestate.musictoggle) {
		emit_stream_np(stream_menutheme);
	}

	assets.cursortexture.load("Textures/Cursor.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });

	assets.Mapcircletexture.load("Textures/MapCircle.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	assets.Mapboxtexture.load("Textures/MapBox.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	assets.Maparrowtexture.load("Textures/MapArrow.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });

	temptexdetail = gamestate.texdetail;
	if (gamestate.texdetail > 2) {
		gamestate.texdetail = 2;
	}
	assets.Mainmenuitems[0].load("Textures/Lugaru.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	assets.Mainmenuitems[1].load("Textures/NewGame.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	assets.Mainmenuitems[2].load("Textures/Options.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	assets.Mainmenuitems[3].load("Textures/Quit.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	assets.Mainmenuitems[4].load("Textures/Eyelid.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	assets.Mainmenuitems[5].load("Textures/Resume.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	assets.Mainmenuitems[6].load("Textures/EndGame.png", 0, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });

	gamestate.texdetail = temptexdetail;

	FadeLoadingScreen(95, gamestate);

	gamestate.gameon = 0;
	gamestate.mainmenu = 1;

	gamestate.stillloading = 0;
	gamestate.firstLoadDone = false;

	gamestate.newdetail = gamestate.detail;
	gamestate.newscreenwidth = gamestate.screenwidth;
	gamestate.newscreenheight = gamestate.screenheight;

	Menu::Load(gamestate, assets, keycapture);

	Animation::loadAll([&]() {Game::LoadingScreen(gamestate, assets); }, assets.graphics);

	PersonType::Load(assets.graphics);

	Person::players.emplace_back(new Person(gamestate, assets));
}

void Game::LoadScreenTexture(GameState& gamestate, GameAssets& assets)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (!assets.screentexture) {
		glGenTextures(1, &assets.screentexture);
	}
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, assets.screentexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, gamestate.kTextureSize, gamestate.kTextureSize, 0);
}

//TODO: move LoadStuff() closer to GameTick.cpp to get rid of various vars shared in Game.hpp
/* Loads models and textures which only needs to be loaded once */
void Game::LoadStuff(GameState& gamestate, GameAssets& assets)
{
	float temptexdetail;
	float viewdistdetail;
	float megascale = 1;

	gamestate.loadtime = 0;

	gamestate.stillloading = 1;

	gamestate.visibleloading = false; //don't use loadscreentexture yet
	assets.loadscreentexture.load("Textures/Fire.jpg", 1, gamestate.trilinear, [&]() {Game::LoadingScreen(gamestate, assets); });
	gamestate.visibleloading = true;

	temptexdetail = gamestate.texdetail;
	gamestate.texdetail = 1;
	assets.text.LoadFontTexture("Textures/Font.png", gamestate.trilinear, [&]() { Game::LoadingScreen(gamestate, assets); });
	assets.text.BuildFont();
	assets.textmono.LoadFontTexture("Textures/FontMono.png", gamestate.trilinear, [&]() { Game::LoadingScreen(gamestate, assets); });
	assets.textmono.BuildFont();
	gamestate.texdetail = temptexdetail;

	viewdistdetail = 2;
	gamestate.viewdistance = 50 * megascale * viewdistdetail;

	if (gamestate.detail == 2) {
		gamestate.texdetail = 1;
		gamestate.kTextureSize = 1024;
	}
	else if (gamestate.detail == 1) {
		gamestate.texdetail = 2;
		gamestate.kTextureSize = 512;
	}
	else {
		gamestate.texdetail = 4;
		gamestate.kTextureSize = 256;
	}

	gamestate.realtexdetail = gamestate.texdetail;

	Weapon::Load(gamestate.trilinear, [&]() { Game::LoadingScreen(gamestate, assets); }, assets);

	assets.terrain->shadowtexture.load("Textures/Shadow.png", 0, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.terrain->bloodtexture.load("Textures/Blood.png", 0, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.terrain->breaktexture.load("Textures/Break.png", 0, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.terrain->bloodtexture2.load("Textures/Blood.png", 0, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });

	assets.terrain->footprinttexture.load("Textures/Footprint.png", 0, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.terrain->bodyprinttexture.load("Textures/Bodyprint.png", 0, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.hawktexture.load("Textures/Hawk.png", 0, gamestate.trilinear, [&]() { LoadingScreen(gamestate, assets); });

	assets.cloudtexture.load("Textures/Cloud.png", 1, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.cloudimpacttexture.load("Textures/CloudImpact.png", 1, gamestate.trilinear, [&]() { LoadingScreen(gamestate, assets); });
	assets.bloodtexture.load("Textures/BloodParticle.png", 1, gamestate.trilinear, [&]() { LoadingScreen(gamestate, assets); });
	assets.snowflaketexture.load("Textures/SnowFlake.png", 1, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.flametexture.load("Textures/Flame.png", 1, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.bloodflametexture.load("Textures/BloodFlame.png", 1, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.smoketexture.load("Textures/Smoke.png", 1, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.shinetexture.load("Textures/Shine.png", 1, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.splintertexture.load("Textures/Splinter.png", 1, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.leaftexture.load("Textures/Leaf.png", 1, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });
	assets.toothtexture.load("Textures/Tooth.png", 1, gamestate.trilinear, [&]() {LoadingScreen(gamestate, assets); });

	gamestate.yaw = 0;
	gamestate.pitch = 0;
	ReSizeGLScene(90, .01, gamestate);

	gamestate.viewer = 0;

	//Set up distant light
	gamestate.light.color[0] = .95;
	gamestate.light.color[1] = .95;
	gamestate.light.color[2] = 1;
	gamestate.light.ambient[0] = .2;
	gamestate.light.ambient[1] = .2;
	gamestate.light.ambient[2] = .24;
	gamestate.light.location.x = 1;
	gamestate.light.location.y = 1;
	gamestate.light.location.z = -.2;
	Normalise(&gamestate.light.location);

	LoadingScreen(gamestate, assets);

	SetUpLighting(gamestate);

	gamestate.fadestart = .6;
	gamestate.gravity = -10;

	gamestate.texscale = .2 / megascale / viewdistdetail;
	assets.terrain->scale = 3 * megascale * viewdistdetail;

	gamestate.viewer.x = assets.terrain->size / 2 * assets.terrain->scale;
	gamestate.viewer.z = assets.terrain->size / 2 * assets.terrain->scale;

	assets.hawk.load("Models/Hawk.solid", [&]() { LoadingScreen(gamestate, assets); });
	assets.hawk.Scale(.03, .03, .03);
	assets.hawk.Rotate(90, 1, 1);
	assets.hawk.CalculateNormals(0, [&]() { LoadingScreen(gamestate, assets); });
	assets.hawk.ScaleNormals(-1, -1, -1);
	gamestate.hawkcoords.x = assets.terrain->size / 2 * assets.terrain->scale - 5 - 7;
	gamestate.hawkcoords.z = assets.terrain->size / 2 * assets.terrain->scale - 5 - 7;
	gamestate.hawkcoords.y = assets.terrain->getHeight(gamestate.hawkcoords.x, gamestate.hawkcoords.z) + 25;

	assets.eye.load("Models/Eye.solid", [&]() { Game::LoadingScreen(gamestate, assets); });
	assets.eye.Scale(.03, .03, .03);
	assets.eye.CalculateNormals(0, [&]() { LoadingScreen(gamestate, assets); });

	// cornea and iris are loaded here but nothing ever draws them: grep finds no
	// reader for either name anywhere outside this block. That is not dead code
	// to be deleted - it is the eyeball rendering missing its second half, and
	// the load cost should be paid again only once something draws them.
	// Restoring it means drawing eye + cornea + iris together at the camera's
	// near plane in Game::DrawGLScene, at the same .03 scale. Do not remove the
	// loads on the assumption they are unused; see Docs/FINDINGS.md.
	assets.cornea.load("Models/Cornea.solid", [&]() { Game::LoadingScreen(gamestate, assets); });
	assets.cornea.Scale(.03, .03, .03);
	assets.cornea.CalculateNormals(0, [&]() { LoadingScreen(gamestate, assets); });

	assets.iris.load("Models/Iris.solid", [&]() { Game::LoadingScreen(gamestate, assets); });
	assets.iris.Scale(.03, .03, .03);
	assets.iris.CalculateNormals(0, [&]() { LoadingScreen(gamestate, assets); });

	LoadSave("Textures/WolfBloodFur.png", &assets.graphics.types[wolftype].bloodText[0], gamestate, assets);
	LoadSave("Textures/BloodFur.png", &assets.graphics.types[rabbittype].bloodText[0], gamestate, assets);

	gamestate.oldenvironment = -4;

	gamestate.gameon = 1;
	gamestate.mainmenu = 0;

	//Fix knife stab, too lazy to do it manually
	Vector3 moveamount;
	moveamount = 0;
	moveamount.z = 2;
	// FIXME - Why this uses skeleton.joints.size() and not Animation::numjoints? (are they equal?)
	// It seems skeleton.joints.size() is 0 at this point, so this is useless.
	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		for (unsigned j = 0; j < assets.graphics.animations[knifesneakattackanim].frames.size(); j++) {
			assets.graphics.animations[knifesneakattackanim].frames[j].joints[i].position += moveamount;
		}
	}

	LoadingScreen(gamestate, assets);

	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		for (unsigned j = 0; j < assets.graphics.animations[knifesneakattackedanim].frames.size(); j++) {
			assets.graphics.animations[knifesneakattackedanim].frames[j].joints[i].position += moveamount;
		}
	}

	LoadingScreen(gamestate, assets);

	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		assets.graphics.animations[dead1anim].frames[1].joints[i].position = assets.graphics.animations[dead1anim].frames[0].joints[i].position;
		assets.graphics.animations[dead2anim].frames[1].joints[i].position = assets.graphics.animations[dead2anim].frames[0].joints[i].position;
		assets.graphics.animations[dead3anim].frames[1].joints[i].position = assets.graphics.animations[dead3anim].frames[0].joints[i].position;
		assets.graphics.animations[dead4anim].frames[1].joints[i].position = assets.graphics.animations[dead4anim].frames[0].joints[i].position;
	}
	assets.graphics.animations[dead1anim].frames[0].speed = 0.001;
	assets.graphics.animations[dead2anim].frames[0].speed = 0.001;
	assets.graphics.animations[dead3anim].frames[0].speed = 0.001;
	assets.graphics.animations[dead4anim].frames[0].speed = 0.001;

	assets.graphics.animations[dead1anim].frames[1].speed = 0.001;
	assets.graphics.animations[dead2anim].frames[1].speed = 0.001;
	assets.graphics.animations[dead3anim].frames[1].speed = 0.001;
	assets.graphics.animations[dead4anim].frames[1].speed = 0.001;

	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		for (unsigned j = 0; j < assets.graphics.animations[swordsneakattackanim].frames.size(); j++) {
			assets.graphics.animations[swordsneakattackanim].frames[j].joints[i].position += moveamount;
		}
	}
	LoadingScreen(gamestate, assets);
	for (unsigned j = 0; j < assets.graphics.animations[swordsneakattackanim].frames.size(); j++) {
		assets.graphics.animations[swordsneakattackanim].frames[j].weapontarget += moveamount;
	}

	LoadingScreen(gamestate, assets);

	for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
		for (unsigned j = 0; j < assets.graphics.animations[swordsneakattackedanim].frames.size(); j++) {
			assets.graphics.animations[swordsneakattackedanim].frames[j].joints[i].position += moveamount;
		}
	}

	LoadingScreen(gamestate, assets);

	if (!assets.screentexture) {
		LoadScreenTexture(gamestate, assets);
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

float LoadingClock::advance(float seconds)
{
    if (!primed_) {
        primed_ = true;
        return 0.0f;
    }
    if (!(seconds > 0.0f) || seconds > maxStepSeconds) {
        return 0.0f;
    }
    elapsed_ += seconds;
    return seconds;
}

bool LoadingClock::primed() const
{
    return primed_;
}

void LoadingClock::reset()
{
    elapsed_ = 0.0f;
}

float LoadingClock::elapsed() const
{
    return elapsed_;
}

float LoadingClock::ramp() const
{
    if (elapsed_ >= rampSeconds) {
        return 100.0f;
    }
    return elapsed_ / rampSeconds * 100.0f;
}

float LoadingClock::decayFlash(float flash, float seconds)
{
    if (!(seconds > 0.0f) || seconds > maxStepSeconds) {
        return flash;
    }
    const float decayed = flash - seconds * flashDecayPerSecond;
    if (decayed < 0.0f) {
        return 0.0f;
    }
    if (decayed > flash) {
        return flash;
    }
    return decayed;
}
