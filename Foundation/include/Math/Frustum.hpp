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

#ifndef _Frustum_HPP_
#define _Frustum_HPP_

class Frustum
{
public:
    // 6 planes
    // Six planes of four coefficients each. std::array because the plane index is
    // computed from matrix arithmetic rather than validated, and an out-of-range
    // plane read returned whatever followed rather than a clipped view.
    std::array<std::array<float, 4>, 6> frustum = {};
    // Calculates and sets frustum from projection and modelview matrix
    void SetFrustum(const float projmatrix[16], const float mvmatrix[16]);

    // Can return 
    // 0 - not inside
    // 1 - partially inside
    // 2 - fully inside
    // TODO Not sure about the last two
    int CubeInFrustum(float x, float y, float z, float size) const;
    int CubeInFrustum(float x, float y, float z, float size, float height) const;
    int SphereInFrustum(float x, float y, float z, float radius) const;
};

#endif
