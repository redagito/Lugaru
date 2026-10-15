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

#include "Graphic/Decal.hpp"
#include "Graphic/Models.hpp"

#include "Environment/Terrain.hpp"

Decal::Decal()
    : position()
    , type(shadowdecal)
    , opacity(0)
    , rotation(0)
    , alivetime(0)
    , brightness(0)
{
}

Decal::Decal(Vector3 _position, decal_type _type, float _opacity, float _rotation, float _brightness, int whichx, int whichy, float size, const Terrain& terrain, bool first)
    : position(_position)
    , type(_type)
    , opacity(_opacity)
    , rotation(_rotation)
    , alivetime(0)
    , brightness(_brightness)
{
    float placex, placez;
    placex = (float)whichx * terrain.scale + terrain.scale;
    placez = (float)whichy * terrain.scale;

    texcoords.at(0).at(0) = (placex - position.x) / size / 2.f + .5f;
    texcoords.at(0).at(1) = (placez - position.z) / size / 2.f + .5f;

    vertex.at(0).x = placex;
    vertex.at(0).z = placez;
    vertex.at(0).y = terrain.heightmap[whichx + 1][whichy] * terrain.scale + .01f;

    if (first) {
        placex = (float)whichx * terrain.scale + terrain.scale;
        placez = (float)whichy * terrain.scale + terrain.scale;
    } else {
        placex = (float)whichx * terrain.scale;
        placez = (float)whichy * terrain.scale;
    }

    texcoords.at(1).at(0) = (placex - position.x) / size / 2.f + .5f;
    texcoords.at(1).at(1) = (placez - position.z) / size / 2.f + .5f;

    vertex.at(1).x = placex;
    vertex.at(1).z = placez;
    if (first) {
        vertex.at(1).y = terrain.heightmap[whichx + 1][whichy + 1] * terrain.scale + .01f;
    } else {
        vertex.at(1).y = terrain.heightmap[whichx][whichy] * terrain.scale + .01f;
    }

    placex = (float)whichx * terrain.scale;
    placez = (float)whichy * terrain.scale + terrain.scale;

    texcoords.at(2).at(0) = (placex - position.x) / size / 2.f + .5f;
    texcoords.at(2).at(1) = (placez - position.z) / size / 2.f + .5f;

    vertex.at(2).x = placex;
    vertex.at(2).z = placez;
    vertex.at(2).y = terrain.heightmap[whichx][whichy + 1] * terrain.scale + .01f;

    Vector3 rot;
    if (rotation) {
        for (int i = 0; i < 3; i++) {
            rot.y = 0;
            rot.x = texcoords.at(i).at(0) - .5f;
            rot.z = texcoords.at(i).at(1) - .5f;
            rot = DoRotation(rot, 0, -rotation, 0);
            texcoords.at(i).at(0) = rot.x + .5f;
            texcoords.at(i).at(1) = rot.z + .5f;
        }
    }
}

Decal::Decal(Vector3 _position, decal_type _type, float _opacity, float _rotation, float size, const Model& model, int i, int which)
    : position(_position)
    , type(_type)
    , opacity(_opacity)
    , rotation(_rotation)
    , alivetime(0)
    , brightness(0)
{
    float placex, placez;
    if (which == 0) {
        placex = model.getTriangleVertex(i, 0).x;
        placez = model.getTriangleVertex(i, 0).z;

        texcoords.at(0).at(0) = (placex - position.x) / (size) / 2.f + .5f;
        texcoords.at(0).at(1) = (placez - position.z) / (size) / 2.f + .5f;

        vertex.at(0).x = placex;
        vertex.at(0).z = placez;
        vertex.at(0).y = model.getTriangleVertex(i, 0).y;

        placex = model.getTriangleVertex(i, 1).x;
        placez = model.getTriangleVertex(i, 1).z;

        texcoords.at(1).at(0) = (placex - position.x) / (size) / 2.f + .5f;
        texcoords.at(1).at(1) = (placez - position.z) / (size) / 2.f + .5f;

        vertex.at(1).x = placex;
        vertex.at(1).z = placez;
        vertex.at(1).y = model.getTriangleVertex(i, 1).y;

        placex = model.getTriangleVertex(i, 2).x;
        placez = model.getTriangleVertex(i, 2).z;

        texcoords.at(2).at(0) = (placex - position.x) / (size) / 2.f + .5f;
        texcoords.at(2).at(1) = (placez - position.z) / (size) / 2.f + .5f;

        vertex.at(2).x = placex;
        vertex.at(2).z = placez;
        vertex.at(2).y = model.getTriangleVertex(i, 2).y;
    } else if (which == 1) {
        placex = model.getTriangleVertex(i, 0).y;
        placez = model.getTriangleVertex(i, 0).z;

        texcoords.at(0).at(0) = (placex - position.y) / (size) / 2.f + .5f;
        texcoords.at(0).at(1) = (placez - position.z) / (size) / 2.f + .5f;

        vertex.at(0).x = model.getTriangleVertex(i, 0).x;
        vertex.at(0).z = placez;
        vertex.at(0).y = placex;

        placex = model.getTriangleVertex(i, 1).y;
        placez = model.getTriangleVertex(i, 1).z;

        texcoords.at(1).at(0) = (placex - position.y) / (size) / 2.f + .5f;
        texcoords.at(1).at(1) = (placez - position.z) / (size) / 2.f + .5f;

        vertex.at(1).x = model.getTriangleVertex(i, 1).x;
        vertex.at(1).z = placez;
        vertex.at(1).y = placex;

        placex = model.getTriangleVertex(i, 2).y;
        placez = model.getTriangleVertex(i, 2).z;

        texcoords.at(2).at(0) = (placex - position.y) / (size) / 2.f + .5f;
        texcoords.at(2).at(1) = (placez - position.z) / (size) / 2.f + .5f;

        vertex.at(2).x = model.getTriangleVertex(i, 2).x;
        vertex.at(2).z = placez;
        vertex.at(2).y = placex;
    } else {
        placex = model.getTriangleVertex(i, 0).x;
        placez = model.getTriangleVertex(i, 0).y;

        texcoords.at(0).at(0) = (placex - position.x) / (size) / 2.f + .5f;
        texcoords.at(0).at(1) = (placez - position.y) / (size) / 2.f + .5f;

        vertex.at(0).x = placex;
        vertex.at(0).z = model.getTriangleVertex(i, 0).z;
        vertex.at(0).y = placez;

        placex = model.getTriangleVertex(i, 1).x;
        placez = model.getTriangleVertex(i, 1).y;

        texcoords.at(1).at(0) = (placex - position.x) / (size) / 2.f + .5f;
        texcoords.at(1).at(1) = (placez - position.y) / (size) / 2.f + .5f;

        vertex.at(1).x = placex;
        vertex.at(1).z = model.getTriangleVertex(i, 1).z;
        vertex.at(1).y = placez;

        placex = model.getTriangleVertex(i, 2).x;
        placez = model.getTriangleVertex(i, 2).y;

        texcoords.at(2).at(0) = (placex - position.x) / (size) / 2.f + .5f;
        texcoords.at(2).at(1) = (placez - position.y) / (size) / 2.f + .5f;

        vertex.at(2).x = placex;
        vertex.at(2).z = model.getTriangleVertex(i, 2).z;
        vertex.at(2).y = placez;
    }
}
