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

#ifndef _GFXRESOURCES_HPP_
#define _GFXRESOURCES_HPP_

// Owns everything that must not outlive the GL context.
//
// The rule in this engine is that GL objects have to be released while the
// context is still current. Enforcing that used to mean relying on two things
// that are easy to get wrong: the order members are declared in inside
// GameAssets, and the scope in main() happening to end before SDL_Quit. Both
// worked, and both were documented at length rather than enforced. GameAssets
// carries paragraphs of comments explaining that its members are declared last
// so they are destroyed first; deleteGame separately reached in and deleted two
// textures by hand.
//
// This makes the rule structural instead of conventional. Everything that owns
// a GL object lives here, and this object's destructor releases all of it in a
// defined order. The order that matters is the reverse of allocation, so that
// anything referring to an earlier allocation is already gone.
//
// It is deliberately not a full renderer. It owns resources, not drawing:
// textures, display lists, buffers and models. Nothing in here knows how to
// draw anything, which is what keeps this a change the whole engine can use
// rather than a rewrite.

#include "Graphic/gamegl.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

class GfxResources
{
public:
    GfxResources();
    ~GfxResources();

    // Not copyable: it owns GL objects, which are one-per-context.
    GfxResources(const GfxResources&) = delete;
    GfxResources& operator=(const GfxResources&) = delete;

    // Releases everything, in reverse order of allocation. Safe to call more
    // than once; the destructor calls it.
    void releaseAll();

    // --- Texture objects ---
    // A texture id, or 0 when there is no context to allocate one in.
    GLuint createTexture();
    void deleteTexture(GLuint id);

    // Takes over a texture that exists already - created before this owner
    // was in place, or by a caller that had to allocate it directly.
    void adoptTexture(GLuint id);

    // --- Display lists ---
    // Fixed-function display lists. Returns 0 when there is no context.
    GLuint createDisplayList();
    void deleteDisplayList(GLuint id);


    // How many GL objects of each kind are outstanding. For diagnostics and
    // for asserting that shutdown really did release everything.
    int outstandingTextureCount() const;
    int outstandingDisplayListCount() const;

private:
    // Allocation order, newest last, so releaseAll can walk it backwards.
    std::vector<GLuint> textures;
    std::vector<GLuint> displayLists;
    // Which of the outstanding objects are already gone, so a delete of an
    // unknown id is ignored rather than releasing a reused name.
    std::map<GLuint, bool> textureLive;
    std::map<GLuint, bool> displayListLive;
};

#endif
