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

#include "Graphic/GfxResources.hpp"

GfxResources::GfxResources() = default;

GfxResources::~GfxResources()
{
    releaseAll();
}

void GfxResources::releaseAll()
{
    // Reverse allocation order, so anything allocated later is gone first.
    for (auto id = textures.rbegin(); id != textures.rend(); ++id) {
        if (textureLive[*id]) {
            glDeleteTextures(1, &*id);
            textureLive[*id] = false;
        }
    }
    textures.clear();
    textureLive.clear();

    for (auto id = displayLists.rbegin(); id != displayLists.rend(); ++id) {
        if (displayListLive[*id]) {
            glDeleteLists(*id, 1);
            displayListLive[*id] = false;
        }
    }
    displayLists.clear();
    displayListLive.clear();


}

GLuint GfxResources::createTexture()
{
    GLuint id = 0;
    glGenTextures(1, &id);
    if (!id) {
        return 0;
    }
    textures.push_back(id);
    textureLive[id] = true;
    return id;
}

void GfxResources::deleteTexture(GLuint id)
{
    if (!id) {
        return;
    }
    const auto found = textureLive.find(id);
    if (found == textureLive.end() || !found->second) {
        // Either never allocated here, or already gone. Releasing a name the
        // driver has since reused would destroy someone else's object.
        return;
    }
    glDeleteTextures(1, &id);
    found->second = false;
}

void GfxResources::adoptTexture(GLuint id)
{
    if (!id || textureLive.count(id)) {
        return;
    }
    textures.push_back(id);
    textureLive[id] = true;
}

GLuint GfxResources::createDisplayList()
{
    const GLuint id = glGenLists(1);
    if (!id) {
        return 0;
    }
    displayLists.push_back(id);
    displayListLive[id] = true;
    return id;
}

void GfxResources::deleteDisplayList(GLuint id)
{
    if (!id) {
        return;
    }
    const auto found = displayListLive.find(id);
    if (found == displayListLive.end() || !found->second) {
        return;
    }
    glDeleteLists(id, 1);
    found->second = false;
}



int GfxResources::outstandingTextureCount() const
{
    int count = 0;
    for (const auto& entry : textureLive) {
        if (entry.second) {
            count++;
        }
    }
    return count;
}

int GfxResources::outstandingDisplayListCount() const
{
    int count = 0;
    for (const auto& entry : displayListLive) {
        if (entry.second) {
            count++;
        }
    }
    return count;
}


