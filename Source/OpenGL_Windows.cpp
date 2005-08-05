#include <vld.h>
#include "Game.h"

// ADDED GWC
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")

extern bool buttons[3];
extern float multiplier;
extern float screenwidth,screenheight;
extern float sps;
extern float realmultiplier;
extern int slomo;
extern bool ismotionblur;
extern float usermousesensitivity;
extern int detail;
extern bool floatjump;
extern bool cellophane;
// MODIFIED GWC
//extern int terraindetail;
//extern int texdetail;
extern float terraindetail;
extern float texdetail;
extern int bloodtoggle;
extern bool osx;
extern bool autoslomo;
extern bool foliage;
extern bool musictoggle;
extern bool trilinear;
extern float gamespeed;
extern int difficulty;
extern bool damageeffects;
extern int numplayers;
extern bool decals;
extern bool invertmouse;
extern bool texttoggle;
extern bool ambientsound;
extern bool mousejump;
extern bool freeze;
extern Person player[maxplayers];
extern bool vblsync;
extern bool stillloading;
extern bool showpoints;
extern bool alwaysblur;
extern bool immediate;
extern bool velocityblur;
extern bool debugmode;
extern int mainmenu;
/*extern*/ bool gameFocused;
extern int kBitsPerPixel;
extern float slomospeed;
extern float slomofreq;
extern float oldgamespeed;
extern float volume;

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include "gl.h"
#include "WinDefs.h"
#include <shellapi.h>
#include "fmod.h"

#include "res/resource.h"

using namespace std;


unsigned int resolutionDepths[8][2] = {0};

bool selectDetail(int & width, int & height, int & bpp, int & detail);
int closestResolution(int width, int height);
int resolutionID(int width, int height);

void ReportError (char * strError);

void SetupDSpFullScreen();
void ShutdownDSp();

void DrawGL(Game & game);

void CreateGLWindow (void);
Boolean SetUp (Game & game);
void DoKey (SInt8 theKey, SInt8 theCode);
void DoUpdate (Game & game);

void DoEvent (void);
void CleanUp (void);


// statics/globals (internal only) ------------------------------------------

Point delta;

static const char g_wndClassName[]={ "LUGARUWINDOWCLASS" };

static HINSTANCE g_appInstance;
static HWND g_windowHandle;
static HGLRC hRC;
static bool g_button, fullscreen = true;


// Menu defs
enum 
{
	kFileQuit = 1
};

enum 
{
	kForegroundSleep = 10,
	kBackgroundSleep = 10000
};


int kContextWidth;
int kContextHeight;

const RGBColor rgbBlack = { 0x0000, 0x0000, 0x0000 };

extern HDC hDC;
GLuint gFontList;
char gcstrMode [256] = "";

UInt32 gSleepTime = kForegroundSleep;
Boolean gDone = false, gfFrontProcess = true;

Game * pgame = 0;

// --------------------------------------------------------------------------

void ReportError (char * strError)
{
	throw std::exception( strError);

	/*	char errMsgCStr [256];
	Str255 strErr;

	sprintf (errMsgCStr, "%s", strError); 

	// out as debug string
	CToPStr (strErr, errMsgCStr);
	DebugStr (strErr);
	*/
}

void SetupDSpFullScreen ()
{
	LOGFUNC;

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset( &dmScreenSettings, 0, sizeof( dmScreenSettings));
		dmScreenSettings.dmSize = sizeof( dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= kContextWidth;
		dmScreenSettings.dmPelsHeight	= kContextHeight;
		dmScreenSettings.dmBitsPerPel	= kBitsPerPixel;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// set video mode
		if (ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			ReportError( "Could not set display mode");
			return;
		}
	}

	ShowCursor(FALSE);
}


void ShutdownDSp ()
{
	LOGFUNC;

	if (fullscreen)
	{
		ChangeDisplaySettings( NULL, 0);
	}

	ShowCursor(TRUE);
}


//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Drawing

void DrawGL (Game & game)
{
	if (hDC == 0)
		return;

	game.DrawGLScene();
}



// --------------------------------------------------------------------------

static Point gMidPoint;

Boolean SetUp (Game & game)
{
	char string[10];

	LOGFUNC;

	randSeed = UpTime().lo;

	osx = 0;
//	ifstream ipstream(":Data:config.txt", std::ios::in /*| std::ios::nocreate*/);
	ifstream ipstream("./Data/config.txt", std::ios::in /*| std::ios::nocreate*/);
	detail=1;
	ismotionblur=0;
	usermousesensitivity=1;
	kContextWidth=640;
	kContextHeight=480;
	kBitsPerPixel = 32;
	floatjump=0;
	cellophane=0;
	texdetail=4;
	autoslomo=1;
	decals=1;
	invertmouse=0;
	bloodtoggle=0;
	terraindetail=2;
	foliage=1;
	musictoggle=1;
	trilinear=1;
	gamespeed=1;
	difficulty=1;
	damageeffects=0;
	texttoggle=1;
	alwaysblur=0;
	showpoints=0;
	immediate=0;
	velocityblur=0;

	slomospeed=0.25;
	slomofreq=8012;

	volume = 0.8f;

	game.crouchkey=MAC_SHIFT_KEY;
	game.jumpkey=MAC_SPACE_KEY;
	game.leftkey=MAC_A_KEY;
	game.forwardkey=MAC_W_KEY;
	game.backkey=MAC_S_KEY;
	game.rightkey=MAC_D_KEY;
	game.drawkey=MAC_E_KEY;
	game.throwkey=MAC_Q_KEY;
	game.attackkey=MAC_MOUSEBUTTON1;
	game.chatkey=MAC_T_KEY;
	numplayers=1;
	ambientsound=1;
	vblsync=0;
	debugmode=0;

	selectDetail(kContextWidth, kContextHeight, kBitsPerPixel, detail);

	if(!ipstream) {
		//ofstream opstream(":Data:config.txt"); 
		ofstream opstream("./Data/config.txt"); 
		opstream << "Screenwidth:\n";
		opstream << kContextWidth;
		opstream << "\nScreenheight:\n";
		opstream << kContextHeight;
		opstream << "\nMouse sensitivity:\n";
		opstream << usermousesensitivity;
		opstream << "\nBlur(0,1):\n";
		opstream << ismotionblur;
		opstream << "\nOverall Detail(0,1,2) higher=better:\n";
		opstream << detail;
		opstream << "\nFloating jump:\n";
		opstream << floatjump;
		opstream << "\nMouse jump:\n";
		opstream << mousejump;
		opstream << "\nAmbient sound:\n";
		opstream << ambientsound;
		opstream << "\nBlood (0,1,2):\n";
		opstream << bloodtoggle;
		opstream << "\nAuto slomo:\n";
		opstream << autoslomo;
		opstream << "\nFoliage:\n";
		opstream << foliage;
		opstream << "\nMusic:\n";
		opstream << musictoggle;
		opstream << "\nTrilinear:\n";
		opstream << trilinear;
		opstream << "\nDecals(shadows,blood puddles,etc):\n";
		opstream << decals;
		opstream << "\nInvert mouse:\n";
		opstream << invertmouse;
		opstream << "\nGamespeed:\n";
		opstream << gamespeed;
		opstream << "\nDifficulty(0,1,2) higher=harder:\n";
		opstream << difficulty;
		opstream << "\nDamage effects(blackout, doublevision):\n";
		opstream << damageeffects;
		opstream << "\nText:\n";
		opstream << texttoggle;
		opstream << "\nDebug:\n";
		opstream << debugmode;
		opstream << "\nVBL Sync:\n";
		opstream << vblsync;
		opstream << "\nShow Points:\n";
		opstream << showpoints;
		opstream << "\nAlways Blur:\n";
		opstream << alwaysblur;
		opstream << "\nImmediate mode (turn on on G5):\n";
		opstream << immediate;
		opstream << "\nVelocity blur:\n";
		opstream << velocityblur;
		opstream << "\nVolume:\n";
		opstream << volume;
		opstream << "\nForward key:\n";
		opstream << KeyToChar(game.forwardkey);
		opstream << "\nBack key:\n";
		opstream << KeyToChar(game.backkey);
		opstream << "\nLeft key:\n";
		opstream << KeyToChar(game.leftkey);
		opstream << "\nRight key:\n";
		opstream << KeyToChar(game.rightkey);
		opstream << "\nJump key:\n";
		opstream << KeyToChar(game.jumpkey);
		opstream << "\nCrouch key:\n";
		opstream << KeyToChar(game.crouchkey);
		opstream << "\nDraw key:\n";
		opstream << KeyToChar(game.drawkey);
		opstream << "\nThrow key:\n";
		opstream << KeyToChar(game.throwkey);
		opstream << "\nAttack key:\n";
		opstream << KeyToChar(game.attackkey);
		opstream << "\nChat key:\n";
		opstream << KeyToChar(game.chatkey);
		opstream.close();
	}
	if(ipstream){
		int i;
		ipstream.ignore(256,'\n');
		ipstream >> kContextWidth;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> kContextHeight;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> usermousesensitivity;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		ismotionblur = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> detail;
		if(detail!=0)kBitsPerPixel=32;
		else kBitsPerPixel=16;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		floatjump = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		mousejump = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		ambientsound = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> bloodtoggle;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		autoslomo = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		foliage = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		musictoggle = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		trilinear = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		decals = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		invertmouse = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> gamespeed;
		oldgamespeed=gamespeed;
		if(oldgamespeed==0){
			gamespeed=1;
			oldgamespeed=1;
		}
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> difficulty;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		damageeffects = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		texttoggle = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		debugmode = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		vblsync = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		showpoints = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		alwaysblur = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		immediate = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		velocityblur = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n'); 
		ipstream >> volume;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n'); 
		ipstream >> string;
		game.forwardkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.backkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.leftkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.rightkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.jumpkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.crouchkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.drawkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.throwkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.attackkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.chatkey=CharToKey(string);
		ipstream.close();

		if(detail>2)detail=2;
		if(detail<0)detail=0;
		if(screenwidth>3000)screenwidth=640;
		if(screenwidth<0)screenwidth=640;
		if(screenheight>3000)screenheight=480;
		if(screenheight<0)screenheight=480;
	}
	if(kBitsPerPixel!=32&&kBitsPerPixel!=16){
		kBitsPerPixel=16;
	}


	selectDetail(kContextWidth, kContextHeight, kBitsPerPixel, detail);

	SetupDSpFullScreen();

	//------------------------------------------------------------------
	// create window
	int x = 0, y = 0;
	RECT r = {0, 0, kContextWidth-1, kContextHeight-1};
	DWORD dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;
	DWORD dwExStyle = WS_EX_APPWINDOW;

	if (fullscreen)
	{
		dwStyle |= WS_POPUP;
	}
	else
	{

		dwStyle |= WS_OVERLAPPEDWINDOW;
		dwExStyle |= WS_EX_WINDOWEDGE;
	}

	AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle);

	if (!fullscreen)
	{
		x = (GetSystemMetrics(SM_CXSCREEN) >> 1) - ((r.right - r.left + 1) >> 1);
		y = (GetSystemMetrics(SM_CYSCREEN) >> 1) - ((r.bottom - r.top + 1) >> 1);
	}

	g_windowHandle=CreateWindowEx(
		dwExStyle,
		g_wndClassName, "Lugaru", dwStyle,
		x, y,
//		kContextWidth, kContextHeight,
		r.right - r.left + 1, r.bottom - r.top + 1,
		NULL,NULL,g_appInstance,NULL );
	if (!g_windowHandle)
	{
		ReportError("Could not create window");
		return false;
	}


	//------------------------------------------------------------------
	// setup OpenGL

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
			1,											// Version Number
			PFD_DRAW_TO_WINDOW |						// Format Must Support Window
			PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
			PFD_DOUBLEBUFFER,							// Must Support Double Buffering
			PFD_TYPE_RGBA,								// Request An RGBA Format
			kBitsPerPixel,								// Select Our Color Depth
			0, 0, 0, 0, 0, 0,							// Color Bits Ignored
			0,											// No Alpha Buffer
			0,											// Shift Bit Ignored
			0,											// No Accumulation Buffer
			0, 0, 0, 0,									// Accumulation Bits Ignored
			16,											// 16Bit Z-Buffer (Depth Buffer)  
			0,											// No Stencil Buffer
			0,											// No Auxiliary Buffer
			PFD_MAIN_PLANE,								// Main Drawing Layer
			0,											// Reserved
			0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC( g_windowHandle)))
		ReportError( "Could not get device context");

	GLuint PixelFormat;
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
	{
		ReportError( "Could not find appropriate pixel format");
		return false;
	}

	if (!DescribePixelFormat(hDC, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
	{
		ReportError( "Could not retrieve pixel format");
		return false;
	}

	if (!SetPixelFormat( hDC, PixelFormat, &pfd))
	{
		ReportError( "Could not set pixel format");
		return false;
	}

	if (!(hRC = wglCreateContext(hDC)))
	{
		ReportError( "Could not create rendering context");
		return false;
	}

	if (!wglMakeCurrent(hDC, hRC))
	{
		ReportError( "Could not activate rendering context");
		return false;
	}

	if (fullscreen)
	{
		// Place the window above all topmost windows
		SetWindowPos( g_windowHandle, HWND_TOPMOST, 0,0,0,0,
			SWP_NOMOVE | SWP_NOSIZE );
	}

	SetForegroundWindow(g_windowHandle);
	SetFocus(g_windowHandle);

	glClear( GL_COLOR_BUFFER_BIT );
	SwapBuffers( hDC );

	// clear all states
	glDisable( GL_ALPHA_TEST);
	glDisable( GL_BLEND);
	glDisable( GL_DEPTH_TEST);
	//	glDisable( GL_DITHER);
	glDisable( GL_FOG);
	glDisable( GL_LIGHTING);
	glDisable( GL_LOGIC_OP);
	glDisable( GL_STENCIL_TEST);
	glDisable( GL_TEXTURE_1D);
	glDisable( GL_TEXTURE_2D);
	glPixelTransferi( GL_MAP_COLOR, GL_FALSE);
	glPixelTransferi( GL_RED_SCALE, 1);
	glPixelTransferi( GL_RED_BIAS, 0);
	glPixelTransferi( GL_GREEN_SCALE, 1);
	glPixelTransferi( GL_GREEN_BIAS, 0);
	glPixelTransferi( GL_BLUE_SCALE, 1);
	glPixelTransferi( GL_BLUE_BIAS, 0);
	glPixelTransferi( GL_ALPHA_SCALE, 1);
	glPixelTransferi( GL_ALPHA_BIAS, 0);

	// set initial rendering states
	glShadeModel( GL_SMOOTH);
	glClearDepth( 1.0f);
	glDepthFunc( GL_LEQUAL);
	glDepthMask( GL_TRUE);
	//	glDepthRange( FRONT_CLIP, BACK_CLIP);
	glEnable( GL_DEPTH_TEST);
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glCullFace( GL_FRONT);
	glEnable( GL_CULL_FACE);
	glEnable( GL_LIGHTING);
//	glEnable( GL_LIGHT_MODEL_AMBIENT);
	glEnable( GL_DITHER);
	glEnable( GL_COLOR_MATERIAL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glAlphaFunc( GL_GREATER, 0.5f);

	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
		iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION ||
		ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION)
	{
		ReportError("DevIL version is different...exiting!\n");
		return false;
	}

	ilInit();
	iluInit();
	ilutInit();

	ilutRenderer(ILUT_OPENGL);

	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

	GLint width = kContextWidth;
	GLint height = kContextHeight;
	gMidPoint.h = width / 2;
	gMidPoint.v = height / 2;
	screenwidth=width;
	screenheight=height;

	game.newdetail=detail;
	game.newscreenwidth=screenwidth;
	game.newscreenheight=screenheight;

	game.InitGame();

	return true;
}


static void DoMouse(Game & game)
{
	static Point lastMouse = {-1,-1};
	Point globalMouse;

	POINT pos;
	GetCursorPos(&pos);
	ScreenToClient(g_windowHandle, &pos);
	globalMouse.h = pos.x;
	globalMouse.v = pos.y;

	if (lastMouse.h == globalMouse.h && lastMouse.v == globalMouse.v)
	{
		game.deltah=0;
		game.deltav=0;
	}
	else
	{
		static Point virtualMouse = {0,0};
		delta = globalMouse;

		delta.h -= lastMouse.h;
		delta.v -= lastMouse.v;
		lastMouse.h = pos.x;
		lastMouse.v = pos.y;

		if(mainmenu||(abs(delta.h)<10*realmultiplier*1000&&abs(delta.v)<10*realmultiplier*1000)){
			game.deltah=delta.h*usermousesensitivity;
			game.deltav=delta.v*usermousesensitivity;
			game.mousecoordh=globalMouse.h;
			game.mousecoordv=globalMouse.v;
		}

		if(!mainmenu)
		{
			if(lastMouse.h>gMidPoint.h+100||lastMouse.h<gMidPoint.h-100||lastMouse.v>gMidPoint.v+100||lastMouse.v<gMidPoint.v-100){
				pos.x = gMidPoint.h;
				pos.y = gMidPoint.v;
				ClientToScreen(g_windowHandle, &pos);
				//SetCursorPos( gMidPoint.h,gMidPoint.v);
				SetCursorPos(pos.x, pos.y);
				lastMouse = gMidPoint;
			}
		}
	}
}



// --------------------------------------------------------------------------

void DoKey (SInt8 theKey, SInt8 theCode)
{
	// do nothing
}

// --------------------------------------------------------------------------



void DoFrameRate (int update)
{	
	static long frames = 0;

	static AbsoluteTime time = {0,0};
	static AbsoluteTime frametime = {0,0};
	AbsoluteTime currTime = UpTime ();
	double deltaTime = (float) AbsoluteDeltaToDuration (currTime, frametime);

	if (0 > deltaTime)	// if negative microseconds
		deltaTime /= -1000000.0;
	else				// else milliseconds
		deltaTime /= 1000.0;

	multiplier=deltaTime;
	if(multiplier<.001)multiplier=.001;
	if(multiplier>10)multiplier=10;
	if(update)frametime = currTime;	// reset for next time interval

	deltaTime = (float) AbsoluteDeltaToDuration (currTime, time);

	if (0 > deltaTime)	// if negative microseconds
		deltaTime /= -1000000.0;
	else				// else milliseconds
		deltaTime /= 1000.0;
	frames++;
	if (0.001 <= deltaTime)	// has update interval passed
	{
		if(update){
			time = currTime;	// reset for next time interval
			frames = 0;
		}
	}
}


void DoUpdate (Game & game)
{
	static float sps=200;
	static int count;
	static float oldmult;

	DoFrameRate(1);
	if(multiplier>.6)multiplier=.6;

	game.fps=1/multiplier;

	count = multiplier*sps;
	if(count<2)count=2;
	//if(count>10)count=10;

	realmultiplier=multiplier;
	multiplier*=gamespeed;
	if(difficulty==1)multiplier*=.9;
	if(difficulty==0)multiplier*=.8;

	if(game.loading==4)multiplier*=.00001;
	//multiplier*.9;
	if(slomo&&!mainmenu)multiplier*=slomospeed;
	//if(freeze)multiplier*=0.00001;
	oldmult=multiplier;
	multiplier/=(float)count;

	DoMouse(game);

	game.TickOnce();

	for(int i=0;i<count;i++)
	{
		game.Tick();
	}
	multiplier=oldmult;

	game.TickOnceAfter();
/* - Debug code to test how many channels were active on average per frame
	static long frames = 0;

	static AbsoluteTime start = {0,0};
	AbsoluteTime currTime = UpTime ();
	static int num_channels = 0;
	
	num_channels += FSOUND_GetChannelsPlaying();
	double deltaTime = (float) AbsoluteDeltaToDuration (currTime, start);

	if (0 > deltaTime)	// if negative microseconds
		deltaTime /= -1000000.0;
	else				// else milliseconds
		deltaTime /= 1000.0;

	++frames;

	if (deltaTime >= 1)
	{
		start = currTime;
		float avg_channels = (float)num_channels / (float)frames;

		ofstream opstream("log.txt",ios::app); 
		opstream << "Average frame count: ";
		opstream << frames;
		opstream << " frames - ";
		opstream << avg_channels;
		opstream << " per frame.\n";
		opstream.close();

		frames = 0;
		num_channels = 0;
	}
*/
	DrawGL (game);
}

// --------------------------------------------------------------------------


void CleanUp (void)
{
	LOGFUNC;

//	game.Dispose();

	ilShutDown();

	if (hRC)
	{
		wglMakeCurrent( NULL, NULL);
		wglDeleteContext( hRC);
		hRC = NULL;
	}

	if (hDC)
	{
		ReleaseDC( g_windowHandle, hDC);
		hDC = NULL;
	}

	if (g_windowHandle)
	{
		ShowWindow( g_windowHandle, SW_HIDE );
		DestroyWindow( g_windowHandle);
		g_windowHandle = NULL;
	}

	ShutdownDSp ();
	ClipCursor(NULL);
}

// --------------------------------------------------------------------------

static bool g_focused = true;


static bool IsFocused()
{
	if (!g_focused)
		return false;

	if (GetActiveWindow() != g_windowHandle)
		return false;

	if (IsIconic( g_windowHandle))
		return false;

	return true;
}


int main (void)
{
	LOGFUNC;

	try
	{
		bool regnow = false;
		{
			Game game;
			pgame = &game;

			//ofstream os("error.txt");
			//os.close();
			//ofstream os("log.txt");
			//os.close();

			SetUp (game);

			while (!gDone&&!game.quit&&(!game.tryquit||!game.registered))
			{
				if (IsFocused())
				{
					gameFocused = true;

					// check windows messages
					MSG msg;
					// message pump
					while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD ) )
					{
						if( msg.message == WM_QUIT )
						{
							gDone=true;
							break;
						}
						else
						{
							TranslateMessage( &msg );
							DispatchMessage( &msg );
						}
					}

					// game
					DoUpdate(game);
				}
				else
				{
					if (gameFocused)
					{
						// allow game chance to pause
						gameFocused = false;
						DoUpdate(game);
					}

					// game is not in focus, give CPU time to other apps by waiting for messages instead of 'peeking'
					MSG msg;
					BOOL bRet;
					//if (GetMessage( &msg, g_windowHandle, 0, 0 ))
					/*if (GetMessage( &msg, NULL, 0, 0 ))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}*/
					if ( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
					{ 
						if (bRet <= 0)
						{
							// handle the error and possibly exit
							gDone=true;
						}
						else
						{
							TranslateMessage(&msg); 
							DispatchMessage(&msg); 
						}
					}
				}
			}

			regnow = game.registernow;
		}
		pgame = 0;

		CleanUp ();
//		if(game.registernow){
		if(regnow)
		{
			char url[100];
			sprintf(url,"http://www.wolfire.com/registerpc.html");
			//			LaunchURL(url);
			ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
		}
		return 0;
	}
	catch (const std::exception& error)
	{
		CleanUp();

		std::string e = "Caught exception: ";
		e += error.what();

		LOG(e, Logger::LOG_ERR);

		MessageBox(g_windowHandle, error.what(), "ERROR", MB_OK | MB_ICONEXCLAMATION);
	}

	CleanUp();

	return -1;
}



	// --------------------------------------------------------------------------


#define MAX_WINKEYS 256
	static unsigned short KeyTable[MAX_WINKEYS]=
	{
		0xffff,  // (0)
			MAC_MOUSEBUTTON1,  // VK_LBUTTON	(1)
			MAC_MOUSEBUTTON2,  // VK_RBUTTON	(2)
			0xffff,  // VK_CANCEL	(3)
			0xffff,  // VK_MBUTTON	(4)
			0xffff,  // (5)
			0xffff,  // (6)
			0xffff,  // (7)
			MAC_DELETE_KEY,  // VK_BACK	(8)
			MAC_TAB_KEY,  // VK_TAB	(9)
			0xffff,  // (10)
			0xffff,  // (11)
			0xffff,  // VK_CLEAR	(12)
			MAC_RETURN_KEY,  // VK_RETURN	(13)
			0xffff,  // (14)
			0xffff,  // (15)
			MAC_SHIFT_KEY,  // VK_SHIFT	(16)
			MAC_CONTROL_KEY,  // VK_CONTROL	(17)
			MAC_OPTION_KEY,  // VK_MENU	(18)
			0xffff,  // VK_PAUSE	(19)
			MAC_CAPS_LOCK_KEY,  // #define VK_CAPITAL	(20)
			0xffff,  // (21)
			0xffff,  // (22)
			0xffff,  // (23)
			0xffff,  // (24)
			0xffff,  // (25)
			0xffff,  // (26)
			MAC_ESCAPE_KEY,  // VK_ESCAPE	(27)
			0xffff,  // (28)
			0xffff,  // (29)
			0xffff,  // (30)
			0xffff,  // (31)
			MAC_SPACE_KEY,  // VK_SPACE	(32)
			MAC_PAGE_UP_KEY,  // VK_PRIOR	(33)
			MAC_PAGE_DOWN_KEY,  // VK_NEXT	(34)
			MAC_END_KEY,  // VK_END	(35)
			MAC_HOME_KEY,  // VK_HOME	(36)
			MAC_ARROW_LEFT_KEY,  // VK_LEFT	(37)
			MAC_ARROW_UP_KEY,  // VK_UP	(38)
			MAC_ARROW_RIGHT_KEY,  // VK_RIGHT	(39)
			MAC_ARROW_DOWN_KEY,  // VK_DOWN	(40)
			0xffff,  // VK_SELECT	(41)
			0xffff,  // VK_PRINT	(42)
			0xffff,  // VK_EXECUTE	(43)
			0xffff,  // VK_SNAPSHOT	(44)
			MAC_INSERT_KEY,  // VK_INSERT	(45)
			MAC_DEL_KEY,  // VK_DELETE	(46)
			0xffff,  // VK_HELP	(47)
			MAC_0_KEY,  // VK_0	(48)
			MAC_1_KEY,  // VK_1	(49)
			MAC_2_KEY,  // VK_2	(50)
			MAC_3_KEY,  // VK_3	(51)
			MAC_4_KEY,  // VK_4	(52)
			MAC_5_KEY,  // VK_5	(53)
			MAC_6_KEY,  // VK_6	(54)
			MAC_7_KEY,  // VK_7	(55)
			MAC_8_KEY,  // VK_8	(56)
			MAC_9_KEY,  // VK_9	(57)
			0xffff,  // (58)
			0xffff,  // (59)
			0xffff,  // (60)
			0xffff,  // (61)
			0xffff,  // (62)
			0xffff,  // (63)
			0xffff,  // (64)
			MAC_A_KEY,  // VK_A	(65)
			MAC_B_KEY,  // VK_B	(66)
			MAC_C_KEY,  // VK_C	(67)
			MAC_D_KEY,  // VK_D	(68)
			MAC_E_KEY,  // VK_E	(69)
			MAC_F_KEY,  // VK_F	(70)
			MAC_G_KEY,  // VK_G	(71)
			MAC_H_KEY,  // VK_H	(72)
			MAC_I_KEY,  // VK_I	(73)
			MAC_J_KEY,  // VK_J	(74)
			MAC_K_KEY,  // VK_K	(75)
			MAC_L_KEY,  // VK_L	(76)
			MAC_M_KEY,  // VK_M	(77)
			MAC_N_KEY,  // VK_N	(78)
			MAC_O_KEY,  // VK_O	(79)
			MAC_P_KEY,  // VK_P	(80)
			MAC_Q_KEY,  // VK_Q	(81)
			MAC_R_KEY,  // VK_R	(82)
			MAC_S_KEY,  // VK_S	(83)
			MAC_T_KEY,  // VK_T	(84)
			MAC_U_KEY,  // VK_U	(85)
			MAC_V_KEY,  // VK_V	(86)
			MAC_W_KEY,  // VK_W	(87)
			MAC_X_KEY,  // VK_X	(88)
			MAC_Y_KEY,  // VK_Y	(89)
			MAC_Z_KEY,  // VK_Z	(90)
			0xffff,  // (91)
			0xffff,  // (92)
			0xffff,  // (93)
			0xffff,  // (94)
			0xffff,  // (95)
			MAC_NUMPAD_0_KEY,  // VK_NUMPAD0	(96)
			MAC_NUMPAD_1_KEY,  // VK_NUMPAD1	(97)
			MAC_NUMPAD_2_KEY,  // VK_NUMPAD2	(98)
			MAC_NUMPAD_3_KEY,  // VK_NUMPAD3	(99)
			MAC_NUMPAD_4_KEY,  // VK_NUMPAD4	(100)
			MAC_NUMPAD_5_KEY,  // VK_NUMPAD5	(101)
			MAC_NUMPAD_6_KEY,  // VK_NUMPAD6	(102)
			MAC_NUMPAD_7_KEY,  // VK_NUMPAD7	(103)
			MAC_NUMPAD_8_KEY,  // VK_NUMPAD8	(104)
			MAC_NUMPAD_9_KEY,  // VK_NUMPAD9	(105)
			MAC_NUMPAD_ASTERISK_KEY,  // VK_MULTIPLY	(106)
			MAC_NUMPAD_PLUS_KEY,  // VK_ADD	(107)
			MAC_NUMPAD_ENTER_KEY,  // VK_SEPARATOR	(108)
			MAC_NUMPAD_MINUS_KEY,  // VK_SUBTRACT	(109)
			MAC_NUMPAD_PERIOD_KEY,  // VK_DECIMAL	(110)
			MAC_NUMPAD_SLASH_KEY,  // VK_DIVIDE	(111)
			MAC_F1_KEY,  // VK_F1	(112)
			MAC_F2_KEY,  // VK_F2	(113)
			MAC_F3_KEY,  // VK_F3	(114)
			MAC_F4_KEY,  // VK_F4	(115)
			MAC_F5_KEY,  // VK_F5	(116)
			MAC_F6_KEY,  // VK_F6	(117)
			MAC_F7_KEY,  // VK_F7	(118)
			MAC_F8_KEY,  // VK_F8	(119)
			MAC_F9_KEY,  // VK_F9	(120)
			MAC_F10_KEY,  // VK_F10	(121)
			MAC_F11_KEY,  // VK_F11	(122)
			MAC_F12_KEY,  // VK_F12	(123)
			0xffff,  // (124)
			0xffff,  // (125)
			0xffff,  // (126)
			0xffff,  // (127)
			0xffff,  // (128)
			0xffff,  // (129)
			0xffff,  // (130)
			0xffff,  // (131)
			0xffff,  // (132)
			0xffff,  // (133)
			0xffff,  // (134)
			0xffff,  // (135)
			0xffff,  // (136)
			0xffff,  // (137)
			0xffff,  // (138)
			0xffff,  // (139)
			0xffff,  // (130)
			0xffff,  // (141)
			0xffff,  // (142)
			0xffff,  // (143)
			0xffff,  // VK_NUMLOCK	(144)
			0xffff,  // VK_SCROLL	(145)
			0xffff,  // (146)
			0xffff,  // (147)
			0xffff,  // (148)
			0xffff,  // (149)
			0xffff,  // (150)
			0xffff,  // (151)
			0xffff,  // (152)
			0xffff,  // (153)
			0xffff,  // (154)
			0xffff,  // (155)
			0xffff,  // (156)
			0xffff,  // (157)
			0xffff,  // (158)
			0xffff,  // (159)
			MAC_SHIFT_KEY,  // VK_LSHIFT	(160)
			MAC_SHIFT_KEY,  // VK_RSHIFT	(161)
			MAC_CONTROL_KEY,  // VK_LCONTROL	(162)
			MAC_CONTROL_KEY,  // VK_RCONTROL	(163)
			MAC_OPTION_KEY,  // VK_LMENU	(164)
			MAC_OPTION_KEY,  // VK_RMENU	(165)
			0xffff,  // (166)
			0xffff,  // (167)
			0xffff,  // (168)
			0xffff,  // (169)
			0xffff,  // (170)
			0xffff,  // (171)
			0xffff,  // (172)
			0xffff,  // (173)
			0xffff,  // (174)
			0xffff,  // (175)
			0xffff,  // (176)
			0xffff,  // (177)
			0xffff,  // (178)
			0xffff,  // (179)
			0xffff,  // (180)
			0xffff,  // (181)
			0xffff,  // (182)
			0xffff,  // (183)
			0xffff,  // (184)
			0xffff,  // (185)
			MAC_SEMICOLON_KEY,  // (186)
			MAC_PLUS_KEY,  // (187)
			MAC_COMMA_KEY,  // (188)
			MAC_MINUS_KEY,  // (189)
			MAC_PERIOD_KEY,  // (190)
			MAC_SLASH_KEY,  // (191)
			MAC_TILDE_KEY,  // (192)
			0xffff,  // (193)
			0xffff,  // (194)
			0xffff,  // (195)
			0xffff,  // (196)
			0xffff,  // (197)
			0xffff,  // (198)
			0xffff,  // (199)
			0xffff,  // (200)
			0xffff,  // (201)
			0xffff,  // (202)
			0xffff,  // (203)
			0xffff,  // (204)
			0xffff,  // (205)
			0xffff,  // (206)
			0xffff,  // (207)
			0xffff,  // (208)
			0xffff,  // (209)
			0xffff,  // (210)
			0xffff,  // (211)
			0xffff,  // (212)
			0xffff,  // (213)
			0xffff,  // (214)
			0xffff,  // (215)
			0xffff,  // (216)
			0xffff,  // (217)
			0xffff,  // (218)
			MAC_LEFTBRACKET_KEY,  // (219)
			MAC_BACKSLASH_KEY,  // (220)
			MAC_RIGHTBRACKET_KEY,  // (221)
			MAC_APOSTROPHE_KEY,  // (222)
			0xffff,  // (223)
			0xffff,  // (224)
			0xffff,  // (225)
			0xffff,  // (226)
			0xffff,  // (227)
			0xffff,  // (228)
			0xffff,  // (229)
			0xffff,  // (230)
			0xffff,  // (231)
			0xffff,  // (232)
			0xffff,  // (233)
			0xffff,  // (234)
			0xffff,  // (235)
			0xffff,  // (236)
			0xffff,  // (237)
			0xffff,  // (238)
			0xffff,  // (239)
			0xffff,  // (240)
			0xffff,  // (241)
			0xffff,  // (242)
			0xffff,  // (243)
			0xffff,  // (244)
			0xffff,  // (245)
			0xffff,  // (246)
			0xffff,  // (247)
			0xffff,  // (248)
			0xffff,  // (249)
			0xffff,  // (250)
			0xffff,  // (251)
			0xffff,  // (252)
			0xffff,  // (253)
			0xffff,  // (254)
			0xffff,  // (255)
	};


	static KeyMap g_theKeys;

	void SetKey( int key)
	{
		g_theKeys[ key >> 3] |= (1 << (key & 7));
	}

	void ClearKey( int key)
	{
		g_theKeys[ key >> 3] &= (0xff ^ (1 << (key & 7)));
	}

	void GetKeys(  unsigned char theKeys[16])
	{
		memcpy( theKeys, &g_theKeys, 16);
	}

	Boolean Button()
	{
		return g_button;
	}


	void ClipMouseToWindow(HWND window)
	{
		RECT wRect;

		GetClientRect(window, &wRect);

		ClientToScreen(window, (LPPOINT)&wRect.left);
		ClientToScreen(window, (LPPOINT)&wRect.right);

		ClipCursor(&wRect);

		return;
	}

	LRESULT FAR PASCAL AppWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
		/* this is where we receive all messages concerning this window
		* we can either process a message or pass it on to the default
		* message handler of windows */

		static PAINTSTRUCT ps;

		switch(msg)
		{
		case WM_ACTIVATE:	// Watch For Window Activate Message
			{
				// Check Minimization State
				BOOL iconified = HIWORD(wParam) ? TRUE : FALSE;

				if (LOWORD(wParam) == WA_INACTIVE)
				{
					ClipCursor(NULL);

					if (fullscreen)
					{
						if( !iconified )
						{
							// Minimize window
							CloseWindow( hWnd );

							// The window is now iconified
							iconified = GL_TRUE;
						}
					}

					ShutdownDSp();

					g_focused=false;				// Program Is Active
				}
				else
				{
					SetupDSpFullScreen();

					if( iconified )
					{
						// Minimize window
						OpenIcon( hWnd );

						// The window is now iconified
						iconified = GL_FALSE;

						// Activate window
						ShowWindow( hWnd, SW_SHOW );
						SetForegroundWindow( hWnd );
						SetFocus( hWnd );
					}

					ClipMouseToWindow(hWnd);
					g_focused=true;			// Program Is No Longer Active
				}

				return 0;						// Return To The Message Loop
			}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			{
				// check for Alt-F4 (exit hotkey)
				if (wParam == VK_F4)
				{
					if (GetKeyState( VK_MENU) & 0x8080)
					{
						gDone = true;
						break;
					}
				}
				if (wParam < MAX_WINKEYS)
				{
					if (KeyTable[wParam] != 0xffff)
						SetKey( KeyTable[wParam]);
				}
				return (0);
			}

		case WM_KEYUP:
		case WM_SYSKEYUP:
			{
				if (wParam < MAX_WINKEYS)
					if (KeyTable[wParam] != 0xffff)
						ClearKey( KeyTable[wParam]);
				return (0);
			}

		case WM_CHAR:
		case WM_DEADCHAR:
		case WM_SYSCHAR:
		case WM_SYSDEADCHAR:
			return (0);

		case WM_NCLBUTTONDOWN:
		case WM_LBUTTONDOWN:
			{
				g_button = true;
				buttons[ 0] = true;
			}
			return (0);

		case WM_NCRBUTTONDOWN:
		case WM_RBUTTONDOWN:
			{
				buttons[ 1] = true;
			}
			return (0);

		case WM_NCMBUTTONDOWN:
		case WM_MBUTTONDOWN:
			{
				buttons[ 2] = true;
			}
			return (0);

		case WM_NCLBUTTONUP:
		case WM_LBUTTONUP:
			{
				g_button = false;
				buttons[ 0] = false;
			}
			return (0);

		case WM_NCRBUTTONUP:
		case WM_RBUTTONUP:
			{
				buttons[ 1] = false;
			}
			return (0);

		case WM_NCMBUTTONUP:
		case WM_MBUTTONUP:
			{
				buttons[ 2] = false;
			}
			return (0);

		case WM_NCLBUTTONDBLCLK:
		case WM_NCRBUTTONDBLCLK:
		case WM_NCMBUTTONDBLCLK:
		case WM_LBUTTONDBLCLK:
			return (0);
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
			return (0);

		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			/*			((WindowInfo *)g_lastWindow->GetInfo())->m_mouseX = (signed short)(lParam & 0xffff);
			((WindowInfo *)g_lastWindow->GetInfo())->m_mouseY = (signed short)(lParam >> 16);
			if (g_lastWindow->m_mouseCallbacksEnabled) g_lastWindow->MouseMoveCallback();
			*///			goto winmessage;
			return (0);

		case WM_SYSCOMMAND:						// Intercept System Commands
			{
				switch (wParam)						// Check System Calls
				{
				case SC_SCREENSAVE:				// Screensaver Trying To Start?
				case SC_MONITORPOWER:			// Monitor Trying To Enter Powersave?
					return 0;					// Prevent From Happening

					// User trying to access application menu using ALT?
				case SC_KEYMENU:
					return 0;
				}
			}
			break;

		case WM_MOVE:
//			{
//				ReleaseCapture();
//				ClipMouseToWindow(hWnd);
//			}
			break;

		case WM_SIZE:
			break;

		case WM_CLOSE:
			{
				//gDone =  true;
				//game.tryquit=1;
			}
			//return (0);

		case WM_DESTROY:
			{
				//ClipCursor(NULL);
				PostQuitMessage(0);  /* Terminate Application */
			}
			return (0);

		case WM_ERASEBKGND:
			break;

		case WM_PAINT:
//			BeginPaint( g_windowHandle,&ps);
//			EndPaint( g_windowHandle,&ps);
			break;

		default:
			break;
		}

		/* We processed the message and there
		* is no processing by Windows necessary */

		/* We didn't process the message so let Windows do it */
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}


	static BOOL RegisterWindowClasses(HINSTANCE hFirstInstance)
	{
		WNDCLASSEX wc;
		memset( &wc, 0, sizeof( wc));

		/* Register the window class. */
		wc.cbSize = sizeof(wc);
#undef style
		wc.style = (CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC);  /* Combination of Class Styles */
		wc.lpfnWndProc = AppWndProc;       /* Adress of Window Procedure */
		wc.cbClsExtra = 0;                 /* Extra Bytes allocated for this Class */
		wc.cbWndExtra = 0;                 /* Extra Bytes allocated for each Window */
		wc.hInstance = hFirstInstance;     /* Handle of program instance */
		wc.hIcon = LoadIcon( hFirstInstance, MAKEINTRESOURCE(IDI_LUGARU) );
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = g_wndClassName; /* Name of the Window Class */
		wc.hIconSm = LoadIcon( hFirstInstance, MAKEINTRESOURCE(IDI_LUGARU) );

		if (!RegisterClassEx(&wc)) return FALSE;  /* Register Class failed */

		return TRUE;
	}

	int resolutionID(int width, int height)
	{
		int whichres;
		whichres=-1;
		if(width==640 && height==480)whichres=0;
		if(width==800 && height==600)whichres=1;
		if(width==1024 && height==768)whichres=2;
		if(width==1280 && height==1024)whichres=3;
		if(width==1600 && height==1200)whichres=4;
		if(width==840 && height==524)whichres=5;
		if(width==1024 && height==640)whichres=6;
		if(width==1344 && height==840)whichres=7;

		return whichres;
	}

	int closestResolution(int width, int height)
	{
		int whichres;
		whichres=-1;
		if(width>=640 && height>=480)whichres=0;
		if(width>=800 && height>=600)whichres=1;
		if(width>=1024 && height>=768)whichres=2;
		if(width>=1280 && height>=1024)whichres=3;
		if(width>=1600 && height>=1200)whichres=4;
		if(width==840 && height==524)whichres=5;
		if(width==1024 && height==640)whichres=6;
		if(width==1344 && height==840)whichres=7;

		return whichres;
	}

	bool selectDetail(int & width, int & height, int & bpp, int & detail)
	{
		bool res = true;
		int whichres = closestResolution(width, height);

		while (true)
		{
			if(whichres<=0 || whichres>7){
				whichres = 0;
				width=640;
				height=480;
			}
			if(whichres==1){
				width=800;
				height=600;
			}
			if(whichres==2){
				width=1024;
				height=768;
			}
			if(whichres==3){
				width=1280;
				height=1024;
			}
			if(whichres==4){
				width=1600;
				height=1200;
			}
			if(whichres==5){
				width=840;
				height=524;
			}
			if(whichres==6){
				width=1024;
				height=640;
			}
			if(whichres==7){
				width=1344;
				height=840;
			}

			if ((detail != 0) && (resolutionDepths[whichres][1] != 0))
			{
				break;
			}
			else if ((detail == 0) && (resolutionDepths[whichres][0] != 0))
			{
				break;
			}
			else if ((detail != 0) && (resolutionDepths[whichres][0] != 0))
			{
				res = false;
				detail = 0;
				break;
			}
			else if (0 == whichres)
			{
				break;
			}

			--whichres;
		}

		bpp = resolutionDepths[whichres][(detail != 0)];

		return res;
	}

	int __stdcall WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
	{
		int argc = 0;
		LPWSTR * cl = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (argc > 1)
		{
			if (0 == _wcsicmp(cl[1], L"-windowed"))
			{
				fullscreen = false;
			}
		}

		logger.start(true);

		memset( &g_theKeys, 0, sizeof( KeyMap));

		unsigned int i = 0;
		DEVMODE mode;
		memset(&mode, 0, sizeof(mode));
		mode.dmSize = sizeof(mode);
		while (EnumDisplaySettings(NULL, i++, &mode))
		{
			if (mode.dmBitsPerPel < 16)
			{
				continue;
			}

			int res = resolutionID(mode.dmPelsWidth, mode.dmPelsHeight);

			if (res > -1 && res < 8)
			{
				if (DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettings(&mode, CDS_TEST))
				{
					continue;
				}

				switch(mode.dmBitsPerPel)
				{
				case 32:
				case 24:
					resolutionDepths[res][1] = mode.dmBitsPerPel;
					break;
				case 16:
					resolutionDepths[res][0] = mode.dmBitsPerPel;
					break;
				}
			}
		}

		/* if there is no Instance of our program in memory then register the window class */
		if (hPrevInstance == NULL && !RegisterWindowClasses(hInstance))
			return FALSE;  /* registration failed! */

		g_appInstance=hInstance;

		main();

		UnregisterClass( g_wndClassName, hInstance);

		return TRUE;

	}

	extern int channels[100];
	extern FSOUND_SAMPLE * samp[100];
	extern FSOUND_STREAM * strm[10];

	extern "C" void PlaySoundEx(int chan, FSOUND_SAMPLE *sptr, FSOUND_DSPUNIT *dsp, signed char startpaused)
	{
		const FSOUND_SAMPLE * currSample = FSOUND_GetCurrentSample(channels[chan]);
		if (currSample && currSample == samp[chan])
		{
			if (FSOUND_GetPaused(channels[chan]))
			{
				FSOUND_StopSound(channels[chan]);
				channels[chan] = FSOUND_FREE;
			}
			else if (FSOUND_IsPlaying(channels[chan]))
			{
				int loop_mode = FSOUND_GetLoopMode(channels[chan]);
				if (loop_mode & FSOUND_LOOP_OFF)
				{
					channels[chan] = FSOUND_FREE;
				}
			}
		}
		else
		{
			channels[chan] = FSOUND_FREE;
		}

		channels[chan] = FSOUND_PlaySoundEx(channels[chan], sptr, dsp, startpaused);
		if (channels[chan] < 0)
		{
			channels[chan] = FSOUND_PlaySoundEx(FSOUND_FREE, sptr, dsp, startpaused);
		}
	}

	extern "C" void PlayStreamEx(int chan, FSOUND_STREAM *sptr, FSOUND_DSPUNIT *dsp, signed char startpaused)
	{
		const FSOUND_SAMPLE * currSample = FSOUND_GetCurrentSample(channels[chan]);
		if (currSample && currSample == FSOUND_Stream_GetSample(sptr))
		{
				FSOUND_StopSound(channels[chan]);
				FSOUND_Stream_Stop(sptr);
		}
		else
		{
			FSOUND_Stream_Stop(sptr);
			channels[chan] = FSOUND_FREE;
		}

		channels[chan] = FSOUND_Stream_PlayEx(channels[chan], sptr, dsp, startpaused);
		if (channels[chan] < 0)
		{
			channels[chan] = FSOUND_Stream_PlayEx(FSOUND_FREE, sptr, dsp, startpaused);
		}
	}

	bool LoadImage(const char * fname, TGAImageRec & tex)
	{
		bool res = true;

		if ( tex.data == NULL )
		{
			return false;
		}

		ILstring f = strdup(ConvertFileName(fname));
		if (!f)
		{
			return false;
		}

		ILuint iid=0;
		ilGenImages(1, &iid);
		ilBindImage(iid);
		if (ilLoadImage(f))
		{
			//iluFlipImage();
			tex.sizeX = ilGetInteger(IL_IMAGE_WIDTH);
			tex.sizeY = ilGetInteger(IL_IMAGE_HEIGHT);
			tex.bpp = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
			ILuint Bpp = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL),
				imageSize = tex.sizeX * tex.sizeY * Bpp;
			ILubyte *Data = ilGetData();
			memcpy(tex.data, Data, imageSize);

			// Truvision Targa files are stored as BGR colors
			// We want RGB so Blue and Red bytes are switched
			if (IL_TGA == ilGetInteger(IL_IMAGE_FORMAT))
			{
				// Loop Through The Image Data
				for (GLuint i = 0; i < int(imageSize); i += Bpp)
				{
					// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
					GLbyte temp;						// Temporary Variable
					temp = tex.data[i];					// Temporarily Store The Value At Image Data 'i'
					tex.data[i] = tex.data[i + 2];		// Set The 1st Byte To The Value Of The 3rd Byte
					tex.data[i + 2] = temp;				// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
				}
			}
		}
		else
		{
			res = false;
		}
		ilDeleteImages(1, &iid);
/*
		if (tid)
		{
			GLuint texid = ilutGLLoadImage(f);
			*tid = texid;
		}
		else if (mip)
		{
			ilutGLBuildMipmaps()
		}
		else
		{
			ilutGLTexImage(0);
		}
*/
		free(f);

		return res;
	}

	void ScreenShot(const char * fname)
	{
		ILstring f = strdup(fname);
		if (!f)
		{
			return;
		}

		ILuint iid;
		ilGenImages(1, &iid);
		ilBindImage(iid);
		if (ilutGLScreen())
		{
			ilSaveImage(f);
		}
		ilDeleteImages(1, &iid);

		free(f);
	}