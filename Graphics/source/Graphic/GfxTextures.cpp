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

namespace
{

// The implementation in force when nothing has been installed. Everything it
// does is a no-op, which is what makes the data-loading path runnable in a
// test with no setup at all.
class NullGfxTextures : public GfxTextures
{
public:
    GLuint createTexture() override { return 0; }
    void deleteTexture(GLuint) override {}
    void setEnvironmentMode() override {}
    void bindTexture(GLuint) override {}
    void setMinFilter(GLuint, GLint) override {}
    void setMagFilter(GLuint, GLint) override {}
    void setGenerateMipmap(GLuint, bool) override {}
    void setUnpackAlignment(int) override {}
    void upload(GLuint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) override {}
};

NullGfxTextures nullTextures;

// A pointer rather than a value, so the real implementation can live wherever
// the GL context is owned without this header knowing about it.
GfxTextures* active = &nullTextures;

} // namespace

void Gfx::setTextures(GfxTextures* textures)
{
    active = textures ? textures : &nullTextures;
}

GfxTextures& Gfx::textures()
{
    return *active;
}
