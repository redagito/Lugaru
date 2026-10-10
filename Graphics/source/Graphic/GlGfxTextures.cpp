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

#include "Graphic/GfxTextures.hpp"
#include "Graphic/gamegl.hpp"

// The GL implementation, using the fixed-function entry points the engine
// already uses everywhere else. It knows nothing about images or files: it
// moves pixels into the driver and nothing else.
class GlGfxTextures : public GfxTextures
{
public:
    GLuint createTexture() override
    {
        GLuint id = 0;
        glGenTextures(1, &id);
        return id;
    }

    void deleteTexture(GLuint id) override
    {
        if (id) {
            glDeleteTextures(1, &id);
        }
    }

    void setEnvironmentMode() override
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    void bindTexture(GLuint id) override
    {
        glBindTexture(GL_TEXTURE_2D, id);
    }

    void setMinFilter(GLuint id, GLint filter) override
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    }

    void setMagFilter(GLuint id, GLint filter) override
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    }

    void setGenerateMipmap(GLuint id, bool generate) override
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, generate ? GL_TRUE : GL_FALSE);
    }

    void setUnpackAlignment(int alignment) override
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    }

    void upload(GLuint id, GLsizei width, GLsizei height, GLint internalFormat,
        GLenum format, GLenum type, const void* data) override
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
    }
};

// Exists so main() can install it. Everything else touches it through Gfx::.
GlGfxTextures& glTextures()
{
    static GlGfxTextures instance;
    return instance;
}

void Gfx::installGlTextures()
{
    Gfx::setTextures(&glTextures());
}
