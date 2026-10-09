/*
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

#ifndef _GRAPHICSSTATE_HPP_
#define _GRAPHICSSTATE_HPP_

#include "Animation/Animation.hpp"
#include "Objects/PersonType.hpp"

// Owner of the session-long mutable state that belongs to the Graphics layer.
// The App layer cannot reach into Graphics statics without inverting the
// layering, and GameAssets - which owns the GL-touching members - may depend
// on Graphics but not the other way round, so the Graphics state gets an owner
// of its own that App is allowed to hold, the same way AudioState owns the
// Audio layer's. Held by value in GameAssets and passed down by reference.
struct GraphicsState
{
    std::vector<Animation> animations;
    std::vector<PersonType> types;
};

#endif // _GRAPHICSSTATE_HPP_
