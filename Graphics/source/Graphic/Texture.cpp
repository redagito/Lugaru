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

#include "Graphic/Texture.hpp"

#include "Graphic/GfxTextures.hpp"
#include "Utils/Folders.hpp"
#include "Utils/ImageIO.hpp"
#include "Utils/Log.hpp"

void TextureRes::load(bool trilinear, ProgressCallback callback)
{
    ImageRec texture;

    //load image into 'texture'
    if (!load_image(filename.c_str(), texture, callback)) {
        Log::error("Texture " + filename + " loading failed");
        return;
    }

    skinsize = texture.sizeX;
    GLuint type = GL_RGBA;
    if (texture.bpp == 24) {
        type = GL_RGB;
    }

    Gfx::textures().setUnpackAlignment(1);

    // Everything from here on is context-scoped work, so it goes through the
    // seam rather than straight at GL.
    Gfx::textures().deleteTexture(id);
    id = Gfx::textures().createTexture();
    if (!id) {
        // No context, so there is nothing to bind to. The decoded pixels are
        // still wanted: a skin has to hand them back to its Skeleton.
        storeDecodedPixels(texture, type);
        return;
    }

    Gfx::textures().setEnvironmentMode();
    Gfx::textures().bindTexture(id);
    Gfx::textures().setMagFilter(id, GL_LINEAR);
    if (hasMipmap) {
        Gfx::textures().setMinFilter(id, trilinear ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
        Gfx::textures().setGenerateMipmap(id, true);
    } else {
        Gfx::textures().setMinFilter(id, GL_LINEAR);
    }

    if (isSkin) {
        storeDecodedPixels(texture, type);
        Gfx::textures().upload(id, texture.sizeX, texture.sizeY, type, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else {
        Gfx::textures().upload(id, texture.sizeX, texture.sizeY, type, type, GL_UNSIGNED_BYTE, texture.data);
    }
}

// Strips the alpha channel out of a decoded image, which is what a skin wants:
// the texture itself is opaque and the transparency comes from a separate mask.
void TextureRes::storeDecodedPixels(ImageRec& texture, GLuint type)
{
    free(data);
    const int nb = texture.sizeY * texture.sizeX * (texture.bpp / 8);
    data = (GLubyte*)malloc(nb * sizeof(GLubyte));
    datalen = 0;
    for (int i = 0; i < nb; i++) {
        if ((i + 1) % 4 || type == GL_RGB) {
            data[datalen++] = texture.data[i];
        }
    }
}

void TextureRes::bind()
{
    Gfx::textures().bindTexture(id);
}

TextureRes::TextureRes(const std::string& _filename, bool _hasMipmap, bool trilinear, ProgressCallback callback)
    : id(0)
    , filename(_filename)
    , hasMipmap(_hasMipmap)
    , isSkin(false)
    , skinsize(0)
    , data(NULL)
    , datalen(0)
{
    load(trilinear, callback);
}

TextureRes::TextureRes(const std::string& _filename, bool _hasMipmap, GLubyte* array, int* skinsizep, bool trilinear, ProgressCallback callback)
    : id(0)
    , filename(_filename)
    , hasMipmap(_hasMipmap)
    , isSkin(true)
    , skinsize(0)
    , data(NULL)
    , datalen(0)
{
    load(trilinear, callback);
    *skinsizep = skinsize;
    for (int i = 0; i < datalen; i++) {
        array[i] = data[i];
    }
}

TextureRes::~TextureRes()
{
    free(data);
    Gfx::textures().deleteTexture(id);
}

Texture::Texture()
    : tex(nullptr)
{
}

void Texture::load(const std::string& filename, bool hasMipmap, bool trilinear, ProgressCallback callback)
{
    tex.reset(new TextureRes(Folders::getResourcePath(filename), hasMipmap, trilinear, callback));
}

void Texture::load(const std::string& filename, bool hasMipmap, GLubyte* array, int* skinsizep, bool trilinear, ProgressCallback callback)
{
    tex.reset(new TextureRes(Folders::getResourcePath(filename), hasMipmap, array, skinsizep, trilinear, callback));
}

void Texture::bind()
{
    if (tex) {
        tex->bind();
    } else {
        Gfx::textures().bindTexture(0);
    }
}

void Texture::regenerate(GLsizei width, GLsizei height, const GLubyte* pixels)
{
    if (!tex) {
        return;
    }
    // The skin is an RGB texture, which is what this path exists for.
    Gfx::textures().setGenerateMipmap(tex->textureId(), true);
    Gfx::textures().upload(tex->textureId(), width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}
