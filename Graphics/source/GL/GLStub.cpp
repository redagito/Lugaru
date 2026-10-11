/*
Copyright (C) 2016-2017 - Lugaru contributors (see AUTHORS file)

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

// A stand-in for OpenGL, so the simulation can run with no driver and no
// context.
//
// WHY THIS EXISTS, AND WHAT IT IS NOT
//
// This is a test scaffold, not a rendering layer. It exists so that a level can
// be loaded and ticked headlessly, which the simulation tests need and which no
// amount of interface work could give them on its own. It is deliberately NOT
// the eventual answer to "how is rendering separated from simulation". That
// answer is a real renderer interface with its own implementations; this file is
// a way to postpone needing one without giving up the ability to run the game.
// When that separation lands, this file should be deletable in one commit.
//
// THE RULE FOR STUBBING
//
// The stubs emulate a context that WORKS and whose output is discarded. They do
// not emulate an empty context. That distinction matters: if glGenLists handed
// back 0, every "is this allocated yet" check in the engine would take its
// not-loaded branch and the code being exercised would not be the code that
// ships. A plausible answer means the engine runs its real control flow and only
// the rasterisation is missing.
//
// The engine cannot tell the difference between "rendered to a screen" and
// "rendered to nothing", because the only thing that observes it is
// glReadPixels - which no test path reaches. So a working-context lie is safe
// here in a way an empty-context lie would not be.
//
// WHAT THIS CANNOT TELL YOU
//
// A green run here is evidence about the simulation and nothing else. GL misuse
// is invisible, because glGetError always reports success. Leaks are invisible,
// because names come from an endless counter. It is not a substitute for the
// real build, which remains the only place rendering correctness is checked.
//
// KEEPING IT HONEST
//
// The link fails loudly if the engine starts using a GL function that is not
// stubbed, which is the intended way to find out the list needs a new entry.
// Signature drift is the risk that is NOT caught at link time - names are plain
// C, so a wrong parameter list links fine and passes garbage. The signatures
// below are copied from the GL headers for that reason; check them against the
// headers when adding one, and never when adding it alone.
//
// The names are duplicated below rather than taken from <GL/gl.h> because this
// translation unit must not include that header: it declares these functions.

extern "C"
{

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef long long GLsizeiptr;
typedef long long GLintptr;
typedef char GLchar;
typedef long GLintptrARB;

#define GL_ZERO 0
#define GL_ONE 1
#define GL_NO_ERROR 0
#define GL_TRUE 1
#define GL_FALSE 0

// --- Stubs that must answer, because the engine reads the answer back. ---

// Fresh names from an endless counter. A context never runs out of these; an
// implementation that did would be broken, so this does not.
static unsigned int glstubNextName = 1;

unsigned int glGenLists(int range)
{
    const unsigned int base = glstubNextName;
    glstubNextName += (range > 0) ? (unsigned int)range : 1u;
    return base;
}

// Textures were routed through GfxResources, so they never appeared in the scan
// of engine call sites. They are still GL calls and they still need a stub.
void glGenTextures(int n, unsigned int* textures)
{
    if (!textures) {
        return;
    }
    for (int i = 0; i < n; i++) {
        textures[i] = glstubNextName++;
    }
}

void glDeleteTextures(int n, const unsigned int* textures)
{
    (void)n;
    (void)textures;
}

void glTexImage2D(unsigned int target, int level, int internalFormat, int width, int height,
    int border, unsigned int format, unsigned int type, const void* pixels)
{
    (void)target; (void)level; (void)internalFormat; (void)width; (void)height;
    (void)border; (void)format; (void)type; (void)pixels;
}

unsigned int glGetError(void)
{
    return GL_NO_ERROR;
}

// The queries the engine makes, and the answers a working context would give.
// A smaller viewport would clip; these match a normal desktop context closely
// enough that nothing branches differently.
void glGetIntegerv(unsigned int pname, int* params)
{
    if (!params) {
        return;
    }
    int values[4] = { 0, 0, 0, 0 };
    switch (pname) {
    case 0x0BA6: // GL_MAX_TEXTURE_SIZE
        values[0] = 1024;
        break;
    case 0x0D33: // GL_MAX_TEXTURE_UNITS
        values[0] = 8;
        break;
    case 0x0BA2: // GL_MAX_VIEWPORT_DIMS
        values[0] = 4096;
        values[1] = 4096;
        break;
    case 0x0BA4: // GL_MAX_MODELVIEW_STACK_DEPTH
    case 0x0BA5: // GL_MAX_PROJECTION_STACK_DEPTH
        values[0] = 32;
        break;
    default:
        break;
    }
    params[0] = values[0];
    if (pname == 0x0BA2) {
        params[1] = values[1];
    }
    // Anything the engine queries that is not listed here reads back as a
    // plain all-zero answer, which is what a context with nothing configured
    // would report.
}

// The engine asks this for its matrix and lighting state and never for
// anything that has to be a particular value, so a zeroed matrix is a safe
// answer: anything multiplying by it gets the origin, which is where the
// simulation starts anyway.
void glGetFloatv(unsigned int pname, float* params)
{
    (void)pname;
    if (!params) {
        return;
    }
    for (int i = 0; i < 16; i++) {
        params[i] = 0.0f;
    }
}

// --- Everything else: a context that did the work and kept no state. ---

void glAlphaFunc(unsigned int func, float ref) { (void)func; (void)ref; }
void glBegin(unsigned int mode) { (void)mode; }
void glEnd(void) {}
void glBindTexture(unsigned int target, unsigned int texture) { (void)target; (void)texture; }
void glBlendFunc(unsigned int sfactor, unsigned int dfactor) { (void)sfactor; (void)dfactor; }
void glCallLists(int n, unsigned int type, const void* lists) { (void)n; (void)type; (void)lists; }
void glClear(unsigned int mask) { (void)mask; }
void glClearColor(float red, float green, float blue, float alpha) { (void)red; (void)green; (void)blue; (void)alpha; }
void glClearDepth(double depth) { (void)depth; }
void glClearStencil(int s) { (void)s; }
void glColor3f(float red, float green, float blue) { (void)red; (void)green; (void)blue; }
void glColor4f(float red, float green, float blue, float alpha) { (void)red; (void)green; (void)blue; (void)alpha; }
void glColorMask(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) { (void)red; (void)green; (void)blue; (void)alpha; }
void glColorPointer(int size, unsigned int type, int stride, const void* pointer) { (void)size; (void)type; (void)stride; (void)pointer; }
void glCopyTexImage2D(unsigned int target, int level, int internalFormat, int x, int y, int width, int height, int border) { (void)target; (void)level; (void)internalFormat; (void)x; (void)y; (void)width; (void)height; (void)border; }
void glCopyTexSubImage2D(unsigned int target, int level, int xoffset, int yoffset, int x, int y, int width, int height) { (void)target; (void)level; (void)xoffset; (void)yoffset; (void)x; (void)y; (void)width; (void)height; }
void glCullFace(unsigned int mode) { (void)mode; }
void glDeleteLists(unsigned int list, int range) { (void)list; (void)range; }
void glDepthFunc(unsigned int func) { (void)func; }
void glDepthMask(unsigned char flag) { (void)flag; }
void glDisable(unsigned int cap) { (void)cap; }
void glDisableClientState(unsigned int array) { (void)array; }
void glDrawArrays(unsigned int mode, int first, int count) { (void)mode; (void)first; (void)count; }
void glDrawBuffer(unsigned int mode) { (void)mode; }
void glEnable(unsigned int cap) { (void)cap; }
void glEnableClientState(unsigned int array) { (void)array; }
void glFinish(void) {}
void glHint(unsigned int target, unsigned int mode) { (void)target; (void)mode; }
void glInterleavedArrays(unsigned int format, int stride, const void* pointer) { (void)format; (void)stride; (void)pointer; }
void glLightfv(unsigned int light, unsigned int pname, const float* params) { (void)light; (void)pname; (void)params; }
void glLineWidth(float width) { (void)width; }
void glListBase(unsigned int base) { (void)base; }
void glLoadIdentity(void) {}
void glLoadMatrixf(const float* m) { (void)m; }
void glMatrixMode(unsigned int mode) { (void)mode; }
void glNewList(unsigned int list, unsigned int mode) { (void)list; (void)mode; }
void glNormal3f(float nx, float ny, float nz) { (void)nx; (void)ny; (void)nz; }
void glOrtho(double left, double right, double bottom, double top, double zNear, double zFar) { (void)left; (void)right; (void)bottom; (void)top; (void)zNear; (void)zFar; }
void glPixelStorei(unsigned int pname, int param) { (void)pname; (void)param; }
void glPixelTransferi(unsigned int pname, int param) { (void)pname; (void)param; }
void glPointSize(float size) { (void)size; }
void glPopMatrix(void) {}
void glPushMatrix(void) {}
void glReadBuffer(unsigned int mode) { (void)mode; }
void glReadPixels(int x, int y, int width, int height, unsigned int format, unsigned int type, void* pixels) { (void)x; (void)y; (void)width; (void)height; (void)format; (void)type; (void)pixels; }
void glRotatef(float angle, float x, float y, float z) { (void)angle; (void)x; (void)y; (void)z; }
void glScalef(float x, float y, float z) { (void)x; (void)y; (void)z; }
void glShadeModel(unsigned int mode) { (void)mode; }
void glStencilFunc(unsigned int func, int ref, unsigned int mask) { (void)func; (void)ref; (void)mask; }
void glStencilOp(unsigned int fail, unsigned int zfail, unsigned int zpass) { (void)fail; (void)zfail; (void)zpass; }
void glTexCoord2f(float s, float t) { (void)s; (void)t; }
void glTexCoordPointer(int size, unsigned int type, int stride, const void* pointer) { (void)size; (void)type; (void)stride; (void)pointer; }
void glTexEnvf(unsigned int target, unsigned int pname, float param) { (void)target; (void)pname; (void)param; }
void glTexEnvfv(unsigned int target, unsigned int pname, const float* params) { (void)target; (void)pname; (void)params; }
void glTexEnvi(unsigned int target, unsigned int pname, int param) { (void)target; (void)pname; (void)param; }
void glTexParameterf(unsigned int target, unsigned int pname, float param) { (void)target; (void)pname; (void)param; }
void glTexParameteri(unsigned int target, unsigned int pname, int param) { (void)target; (void)pname; (void)param; }
void glTranslatef(float x, float y, float z) { (void)x; (void)y; (void)z; }
void glVertex2i(int x, int y) { (void)x; (void)y; }
void glVertex3f(float x, float y, float z) { (void)x; (void)y; (void)z; }
void glVertexPointer(int size, unsigned int type, int stride, const void* pointer) { (void)size; (void)type; (void)stride; (void)pointer; }
void glViewport(int x, int y, int width, int height) { (void)x; (void)y; (void)width; (void)height; }
void glEndList(void) {}

} // extern "C"
