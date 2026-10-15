#include <array>
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

#ifndef _LIGHTS_HPP_
#define _LIGHTS_HPP_

#include "Graphic/gamegl.hpp"
#include "Math/Vector3.hpp"

class Light
{
public:
    GLint type;
    std::array<GLfloat, 3> color = {};
      std::array<GLfloat, 3> ambient = {};
      int attach;
      Vector3 location;
      void setColors(GLfloat cr, GLfloat cg, GLfloat cb,
                     GLfloat ar, GLfloat ag, GLfloat ab);
};

void SetUpLight(const Light* whichsource, int whichlight);

#endif
