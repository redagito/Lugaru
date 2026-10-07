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
#include "GameAssets.hpp"
#include "GameGlobals.h"
#include "GameState.hpp"
#include "Globals.h"
#include "KeyCapture.hpp"

#include "Audio/AudioState.hpp"
#include "Audio/openal_wrapper.hpp"
#include "CommandLine.hpp"
#include "Graphic/gamegl.hpp"
#include "Platform/Platform.hpp"
#include "User/Settings.hpp"
#include "WindowContext.hpp"
#include "Menu/Menu.hpp"
#include "Version.hpp"

#include <fstream>
#include <iostream>
#include <math.h>
#include <set>
#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace Game;

#ifdef LUGARU_PLATFORM_WINDOWS
#include <shellapi.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

// --------------------------------------------------------------------------

void initGL(GameState& gamestate)
{
	glClear(GL_COLOR_BUFFER_BIT);
	swap_gl_buffers(WindowContext::mainWindow());

	// clear all states
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_LOGIC_OP);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
	glPixelTransferi(GL_RED_SCALE, 1);
	glPixelTransferi(GL_RED_BIAS, 0);
	glPixelTransferi(GL_GREEN_SCALE, 1);
	glPixelTransferi(GL_GREEN_BIAS, 0);
	glPixelTransferi(GL_BLUE_SCALE, 1);
	glPixelTransferi(GL_BLUE_BIAS, 0);
	glPixelTransferi(GL_ALPHA_SCALE, 1);
	glPixelTransferi(GL_ALPHA_BIAS, 0);

	// set initial rendering states
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_DITHER);
	glEnable(GL_COLOR_MATERIAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glAlphaFunc(GL_GREATER, 0.5f);

	if (CanInitStereo(gamestate.stereomode)) {
		InitStereo(gamestate.stereomode, WindowContext::kContextWidth, WindowContext::kContextHeight);
	}
	else {
		fprintf(stderr, "Failed to initialize stereo, disabling.\n");
		gamestate.stereomode = stereoNone;
	}
}

// --------------------------------------------------------------------------

static Point gMidPoint;

bool SetUp(GameState& gamestate, GameAssets& assets, KeyCapture& keycapture)
{
	gamestate.cellophane = 0;
	gamestate.texdetail = 4;
	gamestate.slomospeed = 0.25;
	slomofreq = 8012;

	DefaultSettings(gamestate);

	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (SDL_Init(SDL_INIT_VIDEO) == -1) {
			fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
			return false;
		}
	}
	if (!LoadSettings(gamestate)) {
		fprintf(stderr, "Failed to load config, creating default\n");
		SaveSettings(gamestate);
	}

	if (SDL_GL_LoadLibrary(NULL) == -1) {
		fprintf(stderr, "SDL_GL_LoadLibrary() failed: %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	for (int displayIdx = 0; displayIdx < SDL_GetNumVideoDisplays(); ++displayIdx) {
		for (int i = 0; i < SDL_GetNumDisplayModes(displayIdx); ++i) {
			SDL_DisplayMode mode;
			if (SDL_GetDisplayMode(displayIdx, i, &mode) == -1) {
				continue;
			}
			if ((mode.w < 640) || (mode.h < 480)) {
				continue; // sane lower limit.
			}
			pair<int, int> resolution(mode.w, mode.h);
			WindowContext::resolutions.insert(resolution);
		}
	}

	if (WindowContext::resolutions.empty()) {
		const std::string error = "No suitable video resolutions found.";
		std::cerr << error << std::endl;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Lugaru init failed!", error.c_str(), NULL);
		SDL_Quit();
		return false;
	}

	if (commandLineOptions[SHOWRESOLUTIONS]) {
		printf("Available resolutions:\n");
		for (auto resolution = WindowContext::resolutions.begin(); resolution != WindowContext::resolutions.end(); resolution++) {
			printf("  %d x %d\n", (int)resolution->first, (int)resolution->second);
		}
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

	// TODO High DPI fix necessary?
	Uint32 sdlflags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN; // | SDL_WINDOW_ALLOW_HIGHDPI;
	if (commandLineOptions[FULLSCREEN]) {
		gamestate.fullscreen = commandLineOptions[FULLSCREEN].last()->type();
	}
	if (gamestate.fullscreen) {
		sdlflags |= SDL_WINDOW_FULLSCREEN;
	}
	if (!commandLineOptions[NOMOUSEGRAB].last()->type()) {
		sdlflags |= SDL_WINDOW_INPUT_GRABBED;
	}

	if (!WindowContext::createWindow(WindowContext::kContextWidth, WindowContext::kContextHeight, sdlflags)) {
		fprintf(stderr, "SDL_CreateWindow() failed: %s\n", SDL_GetError());
		fprintf(stderr, "forcing 640x480...\n");
		WindowContext::kContextWidth = 640;
		WindowContext::kContextHeight = 480;
		if (!WindowContext::createWindow(WindowContext::kContextWidth, WindowContext::kContextHeight, sdlflags)) {
			fprintf(stderr, "SDL_CreateWindow() failed: %s\n", SDL_GetError());
			fprintf(stderr, "forcing 640x480 windowed mode...\n");
			sdlflags &= ~SDL_WINDOW_FULLSCREEN;
			if (!WindowContext::createWindow(WindowContext::kContextWidth, WindowContext::kContextHeight, sdlflags)) {
				fprintf(stderr, "SDL_CreateWindow() failed: %s\n", SDL_GetError());
				return false;
			}
		}
	}

	if (!WindowContext::createGLContext()) {
		fprintf(stderr, "SDL_GL_CreateContext() failed: %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	int dblbuf = 0;
	if ((SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &dblbuf) == -1) || (!dblbuf)) {
		fprintf(stderr, "Failed to get a double-buffered context.\n");
		SDL_Quit();
		return false;
	}

	if (SDL_GL_SetSwapInterval(-1) == -1) { // try swap_tear first.
		SDL_GL_SetSwapInterval(1);
	}

	SDL_ShowCursor(0);
	if (!commandLineOptions[NOMOUSEGRAB].last()->type()) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}

	initGL(gamestate);

	GLint width = WindowContext::kContextWidth;
	GLint height = WindowContext::kContextHeight;
	gMidPoint.h = width / 2;
	gMidPoint.v = height / 2;
	gamestate.screenwidth = width;
	gamestate.screenheight = height;

	gamestate.newdetail = gamestate.detail;
	gamestate.newscreenwidth = gamestate.screenwidth;
	gamestate.newscreenheight = gamestate.screenheight;

	/* If saved resolution is not in the list, add it to the list (so that it’s selectable in the options) */
	pair<int, int> startresolution(width, height);
	if (WindowContext::resolutions.find(startresolution) == WindowContext::resolutions.end()) {
		WindowContext::resolutions.insert(startresolution);
	}

	InitGame(gamestate, assets, keycapture);

	return true;
}

static void DoMouse(GameState& gamestate)
{

	if (gamestate.mainmenu || ((abs(gamestate.deltah) < 10 * gamestate.realmultiplier * 1000) && (abs(gamestate.deltav) < 10 * gamestate.realmultiplier * 1000))) {
		gamestate.deltah *= gamestate.usermousesensitivity;
		gamestate.deltav *= gamestate.usermousesensitivity;
		gamestate.mousecoordh += gamestate.deltah;
		gamestate.mousecoordv += gamestate.deltav;
		if (gamestate.mousecoordh < 0) {
			gamestate.mousecoordh = 0;
		}
		else if (gamestate.mousecoordh >= WindowContext::kContextWidth) {
			gamestate.mousecoordh = WindowContext::kContextWidth - 1;
		}
		if (gamestate.mousecoordv < 0) {
			gamestate.mousecoordv = 0;
		}
		else if (gamestate.mousecoordv >= WindowContext::kContextHeight) {
			gamestate.mousecoordv = WindowContext::kContextHeight - 1;
		}
	}
}

void DoFrameRate(GameState& gamestate, int update)
{
	static long frames = 0;

	static AbsoluteTime time = { 0, 0 };
	static AbsoluteTime frametime = { 0, 0 };
	AbsoluteTime currTime = UpTime();
	double deltaTime = (float)AbsoluteDeltaToDuration(currTime, frametime);

	if (0 > deltaTime) { // if negative microseconds
		deltaTime /= -1000000.0;
	}
	else { // else milliseconds
		deltaTime /= 1000.0;
	}

	gamestate.multiplier = deltaTime;
	if (gamestate.multiplier < .001) {
		gamestate.multiplier = .001;
	}
	if (gamestate.multiplier > 10) {
		gamestate.multiplier = 10;
	}
	if (update) {
		frametime = currTime; // reset for next time interval
	}

	deltaTime = (float)AbsoluteDeltaToDuration(currTime, time);

	if (0 > deltaTime) { // if negative microseconds
		deltaTime /= -1000000.0;
	}
	else { // else milliseconds
		deltaTime /= 1000.0;
	}
	frames++;
	if (0.001 <= deltaTime) { // has update interval passed
		if (update) {
			time = currTime; // reset for next time interval
			frames = 0;
		}
	}
}

void DoUpdate(GameState& gamestate, GameAssets& assets, KeyCapture& keycapture)
{
	static float sps = 200;
	static int count;
	static float oldmult;

	DoFrameRate(gamestate, 1);
	if (gamestate.multiplier > .6) {
		gamestate.multiplier = .6;
	}

	gamestate.fps = 1 / gamestate.multiplier;

	count = gamestate.multiplier * sps;
	if (count < 2) {
		count = 2;
	}

	gamestate.realmultiplier = gamestate.multiplier;
	gamestate.multiplier *= gamestate.gamespeed;
	if (gamestate.difficulty == 1) {
		gamestate.multiplier *= .9;
	}
	if (gamestate.difficulty == 0) {
		gamestate.multiplier *= .8;
	}

	if (gamestate.loading == 4) {
		gamestate.multiplier *= .00001;
	}
	if (gamestate.slomo && !gamestate.mainmenu) {
		gamestate.multiplier *= gamestate.slomospeed;
	}
	oldmult = gamestate.multiplier;
	gamestate.multiplier /= (float)count;

	DoMouse(gamestate);

	TickOnce(gamestate);

	for (int i = 0; i < count; i++) {
		Tick(gamestate, assets, keycapture);
	}
	gamestate.multiplier = oldmult;

	TickOnceAfter(gamestate, assets);
	if (gamestate.stereomode == stereoNone) {
		DrawGLScene(stereoCenter, gamestate, assets, keycapture);
	}
	else {
		DrawGLScene(stereoLeft, gamestate, assets, keycapture);
		DrawGLScene(stereoRight, gamestate, assets, keycapture);
	}
}

// --------------------------------------------------------------------------

void CleanUp(void)
{
	delete[] commandLineOptionsBuffer;

	SDL_Quit();
}

// --------------------------------------------------------------------------

#ifndef LUGARU_PLATFORM_WINDOWS
// (code lifted from physfs: http://icculus.org/physfs/ ... zlib license.)
static char* findBinaryInPath(const char* bin, char* envr)
{
	size_t alloc_size = 0;
	char* exe = NULL;
	char* start = envr;
	char* ptr;

	do {
		size_t size;
		ptr = strchr(start, ':'); /* find next $PATH separator. */
		if (ptr) {
			*ptr = '\0';
		}
		size = strlen(start) + strlen(bin) + 2;
		if (size > alloc_size) {
			char* x = (char*)realloc(exe, size);
			if (x == NULL) {
				if (exe != NULL) {
					free(exe);
				}
				return (NULL);
			} /* if */

			alloc_size = size;
			exe = x;
		} /* if */

		/* build full binary path... */
		strcpy(exe, start);
		if ((exe[0] == '\0') || (exe[strlen(exe) - 1] != '/')) {
			strcat(exe, "/");
		}
		strcat(exe, bin);

		if (access(exe, X_OK) == 0) { /* Exists as executable? We're done. */
			strcpy(exe, start);       /* i'm lazy. piss off. */
			return (exe);
		} /* if */

		start = ptr + 1; /* start points to beginning of next element. */
	} while (ptr != NULL);

	if (exe != NULL) {
		free(exe);
	}

	return (NULL); /* doesn't exist in path. */
} /* findBinaryInPath */

char* calcBaseDir(const char* argv0)
{
	/* If there isn't a path on argv0, then look through the $PATH for it. */
	char* retval;
	char* envr;

	if (strchr(argv0, '/')) {
		retval = strdup(argv0);
		if (retval) {
			*((char*)strrchr(retval, '/')) = '\0';
		}
		return (retval);
	}

	envr = getenv("PATH");
	if (!envr) {
		return NULL;
	}
	envr = strdup(envr);
	if (!envr) {
		return NULL;
	}
	retval = findBinaryInPath(argv0, envr);
	free(envr);
	return (retval);
}

static inline void chdirToAppPath(const char* argv0)
{
	char* dir = calcBaseDir(argv0);
	if (dir) {
#if (defined(__APPLE__) && defined(__MACH__))
		// Chop off /Contents/MacOS if it's at the end of the string, so we
		//  land in the base of the app bundle.
		const size_t len = strlen(dir);
		const char* bundledirs = "/Contents/MacOS";
		const size_t bundledirslen = strlen(bundledirs);
		if (len > bundledirslen) {
			char* ptr = (dir + len) - bundledirslen;
			if (strcasecmp(ptr, bundledirs) == 0)
				*ptr = '\0';
		}
#endif
		errno = 0;
		if (chdir(dir) != 0) {
			printf("Error changing dir to '%s' (%s).\n", dir, strerror(errno));
		}
		free(dir);
	}
}
#endif

namespace
{

// Joins the key-capture thread when the enclosing scope ends, however it ends:
// falling off the bottom, an early return, or an exception unwinding past it.
// The thread holds references to gamestate and to keycapture, so it must not
// outlive either, and destruction is the only thing on every one of those paths.
struct JoinKeySelectThreadOnExit
{
	~JoinKeySelectThreadOnExit()
	{
		Menu::joinKeySelectThread();
	}
};

} // namespace

int main(int argc, char** argv)
{
	argc -= (argc > 0);
	argv += (argc > 0); // skip program name argv[0] if present
	option::Stats stats(true, usage, argc, argv);
	if (commandLineOptionsNumber != stats.options_max) {
		std::cerr << "Found incorrect command line option number" << std::endl;
		return 1;
	}
	commandLineOptionsBuffer = new option::Option[stats.buffer_max];
	option::Parser parse(true, usage, argc, argv, commandLineOptions, commandLineOptionsBuffer);

	if (parse.error()) {
		delete[] commandLineOptionsBuffer;
		return 1;
	}

	// Always start by printing the version and info to the stdout
	std::cout << "--------------------------------------------------------------------------\n"
		<< "Lugaru HD: The Rabbit's Foot, by Wolfire Games and the OSS Lugaru project.\n\n"
		<< "Licensed under the GPL 2.0+ and CC-BY-SA 3.0 and 4.0 licenses.\n"
		<< "More information, updates and bug reports at http://osslugaru.gitlab.io\n"
		<< std::endl;

	std::cout << "Version " + VERSION_STRING + " -- " + VERSION_BUILD_TYPE + " build\n"
		<< "--------------------------------------------------------------------------\n"
		<< std::endl;

	if (commandLineOptions[VERSION]) {
		// That was enough, quit.
		delete[] commandLineOptionsBuffer;
		return 0;
	}

	if (commandLineOptions[HELP]) {
		option::printUsage(std::cout, usage);
		delete[] commandLineOptionsBuffer;
		return 0;
	}

	if (option::Option* opt = commandLineOptions[UNKNOWN]) {
		std::cerr << "Unknown option: " << opt->name << "\n";
		option::printUsage(std::cerr, usage);
		delete[] commandLineOptionsBuffer;
		return 1;
	}

	// !!! FIXME: we could use a Win32 API for this.  --ryan.
#ifndef LUGARU_PLATFORM_WINDOWS
	chdirToAppPath(argv[0]);
#endif

#ifdef NDEBUG
	try {
#endif
		{
			// The one and only GameState for this process. Everything below takes
			// it by reference; nothing else constructs one.
			GameState gamestate;

			// The owner of the GL-backed helpers, for the same reason and by the
			// same rule: one per process, passed by reference to everything that
			// draws or loads. Its destructor deletes GL objects, so it has to
			// outlive every frame and go out of scope before SDL_Quit below.
			GameAssets assets;

			// The handshake with the one thread in the process, by the same rule and
			// for the same reason: one per process, passed by reference. It holds
			// atomics, so unlike GameState it cannot live inside one.
			KeyCapture keycapture;

			// Declared after all three objects the thread references, so it is
			// destroyed before any of them: the join cannot be skipped by the
			// early return below or by an exception unwinding out of this block.
			// It does not run before CleanUp(), so ~GameAssets still deletes its
			// GL objects with a current context.
			JoinKeySelectThreadOnExit joinKeySelectThread;

			if (!SetUp(gamestate, assets, keycapture)) {
				delete[] commandLineOptionsBuffer;
				return 42;
			}

			if (commandLineOptions[DEVTOOLS]) {
				gamestate.devtools = true;
			}

			bool gameDone = false;
			bool gameFocused = true;

			srand((int)time(nullptr));

			if (commandLineOptions[CMD].count() > 0) {
				gamestate.devtools = true;
				Menu::startChallengeLevel(1, gamestate, assets);
				for (option::Option* opt = commandLineOptions[CMD]; opt; opt = opt->next()) {
					if (opt->arg && (strlen(opt->arg) > 0)) {
						cmd_dispatch(opt->arg, gamestate, assets);
					}
				}
			}

			while (!gameDone && !gamestate.tryquit) {
				if (WindowContext::isFocused()) {
					gameFocused = true;

					// check windows messages

					gamestate.deltah = 0;
					gamestate.deltav = 0;
					SDL_Event e;
					if (!keycapture.waiting) {
						// message pump
						while (SDL_PollEvent(&e)) {
							if (!WindowContext::sdlEventProc(e, gamestate)) {
								gameDone = true;
								break;
							}
						}
					}

					// game
					DoUpdate(gamestate, assets, keycapture);
				}
				else {
					if (gameFocused) {
						// allow game chance to pause
						gameFocused = false;
						DoUpdate(gamestate, assets, keycapture);
					}

					// game is not in focus, give CPU time to other apps by waiting for messages instead of 'peeking'
					SDL_WaitEvent(0);
				}
			}

			// Joined here rather than left to the guard above, because deleteGame
			// deletes GL objects the thread would be reading. The guard is what
			// covers the paths that never reach this line.
			Menu::joinKeySelectThread();

			deleteGame(gamestate, assets);
		}

		CleanUp();

		return 0;
#ifdef NDEBUG
	}
	catch (const std::exception& error) {
		CleanUp();

		std::string e = "Caught std::exception: ";
		e += error.what();

		std::cerr << e << std::endl;

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception caught", error.what(), NULL);

		return -1;
	}
#endif
}
