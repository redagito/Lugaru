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

#ifndef _GFXTEXTURES_HPP_
#define _GFXTEXTURES_HPP_

// The GL-facing half of texture handling, behind an interface.
//
// A TextureRes has two jobs that have nothing to do with each other: decoding
// an image file into CPU memory, and pushing that memory at the GL driver.
// The first is what a level load needs; the second only makes sense while a GL
// context is current. Keeping them apart means the decoding - and therefore
// the whole data-loading path that touches textures - runs in a test.
//
// This is deliberately narrow. It is the first slice of a renderer seam, and
// the shape is meant to be copied: isolate the context-scoped operations behind
// an interface, default to an implementation that does nothing, and let the
// program install the real one at startup. More slices (meshes, matrices,
// vertex submission) would follow the same pattern if and when they need the
// same separation.
//
// The process default is a no-op implementation, so nothing has to set one up
// before a test can run. The cost of that choice is that the real program must
// install the GL implementation before it draws, and forgetting would render
// an empty scene rather than fail loudly. That trade is worth it here: the
// alternative, threading a context parameter through every one of the ~100
// texture load sites, is far worse.

#include "Graphic/gamegl.hpp"

#include <string>

class GfxTextures
{
public:
    virtual ~GfxTextures() = default;

    // A texture object, or 0 when there is no context to create one in.
    virtual GLuint createTexture() = 0;
    virtual void deleteTexture(GLuint id) = 0;

    // The fixed-function texturing state the engine relies on. There is one
    // environment mode in the whole program and three combinations of filter,
    // so these take the raw values rather than pretending to an enum.
    virtual void setEnvironmentMode() = 0;
    virtual void bindTexture(GLuint id) = 0;
    virtual void setMinFilter(GLuint id, GLint filter) = 0;
    virtual void setMagFilter(GLuint id, GLint filter) = 0;
    virtual void setGenerateMipmap(GLuint id, bool generate) = 0;
    virtual void setUnpackAlignment(int alignment) = 0;

    // Uploads decoded pixel data. `data` is CPU memory owned by the caller.
    virtual void upload(GLuint id, GLsizei width, GLsizei height, GLint internalFormat,
        GLenum format, GLenum type, const void* data) = 0;
};

namespace Gfx
{

// Installs the texture operations to use. Called once, before any texture is
// loaded, by whoever owns the GL context.
void setTextures(GfxTextures* textures);

// Installs the real GL implementation. Convenience for main(), which is the
// only place that should need to know it exists.
void installGlTextures();

// The operations in force. Never null.
GfxTextures& textures();

} // namespace Gfx

#endif
