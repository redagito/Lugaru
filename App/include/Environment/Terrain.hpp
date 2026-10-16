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

#ifndef _TERRAIN_HPP_
#define _TERRAIN_HPP_

#include "Environment/Lights.hpp"

#include <memory>
#include <vector>

// Graphics
#include "Graphic/Decal.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"

// Foundation
#include "Math/Frustum.hpp"
#include "Math/Vector3.hpp"
#include "Utils/ImageIO.hpp"

#define max_terrain_size 256
#define curr_terrain_size size
#define subdivision 64
#define max_patch_elements (max_terrain_size / subdivision) * (max_terrain_size / subdivision) * 54

#define allfirst 0
#define mixed 1
#define allsecond 2

#define max_decals 1000

#define snowyenvironment 0
#define grassyenvironment 1
#define desertenvironment 2

//
// Model Structures
//

class Terrain
{
public:
    Texture bloodtexture;
    Texture bloodtexture2;
    Texture shadowtexture;
    Texture footprinttexture;
    Texture bodyprinttexture;
    Texture breaktexture;
    short size;

    // The terrain's data is ~22 MB of fixed arrays. Each one is owned here and
    // viewed through a matching raw pointer, so the object stays small enough
    // to sit on the stack and every existing two-dimensional translation still
    // compiles untouched.
    std::unique_ptr<GLfloat[]> vArray_storage;
    GLfloat* vArray = nullptr;

    // Which objects are in each patch, so a query only tests the objects that can
    // possibly be near. std::array with checked access rather than a pointer into a
    // flat block: this is the table that was indexed by an unclamped coordinate and
    // produced an index of any magnitude, and the entries are ids read from the
    // level file.
    //
    // Note the capacity limit of 300 per patch, enforced in AddObject and matched
    // by nothing else.
    std::unique_ptr<std::array<std::array<std::vector<unsigned int>, subdivision>, subdivision>> patchobjects;

    // A tile grid of heights, one row and column larger than the terrain itself
    // because the patch code deliberately reads one past the last tile. Kept as
    // std::array rather than a C array so every access is bounds-checked: this is
    // the array that Decal::Decal indexed with an unclamped tile index, and the
    // one UpdateVertexArray read past the end of.
    std::unique_ptr<std::array<std::array<float, max_terrain_size + 1>, max_terrain_size + 1>> heightmap;

    // The rest of the same family. Every one of these was reachable past its end
    // by the patch and decal code; std::array plus .at() turns that into a throw.
    using HeightRow = std::array<float, max_terrain_size + 1>;
    using NormalRow = std::array<Vector3, max_terrain_size + 1>;
    using PatchRow = std::array<int, subdivision>;
    using PatchScalarRow = std::array<float, subdivision>;

    std::unique_ptr<std::array<NormalRow, max_terrain_size + 1>> normals;

    // One grid cell per triangle pair: (n-1) * (n-1) cells, two triangles each.
    std::unique_ptr<std::array<std::array<Vector3, 3>, (max_terrain_size - 1) * (max_terrain_size - 1) * 2>> triangles;

    std::unique_ptr<std::array<std::array<Vector3, max_terrain_size>, max_terrain_size>> facenormals;

    // RGBA per tile.
    std::unique_ptr<std::array<std::array<std::array<float, 4>, max_terrain_size + 1>, max_terrain_size + 1>> colors;

    std::unique_ptr<std::array<HeightRow, max_terrain_size + 1>> opacityother;
    std::unique_ptr<std::array<HeightRow, max_terrain_size + 1>> texoffsetx;
    std::unique_ptr<std::array<HeightRow, max_terrain_size + 1>> texoffsety;

    std::unique_ptr<std::array<PatchRow, subdivision>> numtris;
    std::unique_ptr<std::array<PatchRow, subdivision>> textureness;
    std::unique_ptr<std::array<std::array<bool, subdivision>, subdivision>> visible;
    std::unique_ptr<std::array<PatchScalarRow, subdivision>> avgypatch;
    std::unique_ptr<std::array<PatchScalarRow, subdivision>> maxypatch;
    std::unique_ptr<std::array<PatchScalarRow, subdivision>> minypatch;
    std::unique_ptr<std::array<PatchScalarRow, subdivision>> heightypatch;

    float scale;
    int type;

    int patch_elements;

    std::vector<Decal> decals;

    void AddObject(Vector3 where, float radius, int id);
    void DeleteObject(unsigned int id);
    void MakeDecal(decal_type decaltype, Vector3 where, float decalradius, float opacity, float rotation, int environment);

    int lineTerrain(Vector3 p1, Vector3 p2, Vector3* p) const;
    float getHeight(float pointx, float pointz) const;
    float getOpacity(float pointx, float pointz) const;
    Vector3 getLighting(float pointx, float pointz) const;
    Vector3 getNormal(float pointx, float pointz) const;
    
    void UpdateVertexArray(int whichx, int whichy, float texscale);
    bool load(const std::string& fileName, int environment, ProgressCallback callback);
    void CalculateNormals();
    void drawdecals(const Vector3& viewer, float viewdistance, float fadestart, float multiplier);
    void draw(int layer, const Vector3& viewer, float viewdistance, float fadestart, int environment, const Frustum& frustum, float blurness);
    void DoShadows(bool tutorialActive, float texscale, const Light& light, bool skyboxtexture, ProgressCallback callback);
    void deleteDeadDecals();

    float getHeightByTile(int x, int y) const;
    Terrain();

    // The patch a world coordinate falls in, clamped to one that exists. A
    // coordinate outside the terrain produced an index past the patch table,
    // and every site that indexes it with a coordinate does so without a
    // bounds check, so the index has to be right where it is computed.
    int patchFor(float coordinate) const;

private:
    void DeleteDecal(int which);
    void MakeDecalLock(decal_type decaltype, Vector3 where, int whichx, int whichy, float decalradius, float opacity, float rotation, int environment);

    void drawpatch(int whichx, int whichy, float opacity, const Vector3& viewer, float viewdistance, float fadestart);
    void drawpatchother(int whichx, int whichy, float opacity, const Vector3& viewer, float viewdistance, float fadestart);
    void drawpatchotherother(int whichx, int whichy, const Vector3& viewer, float viewdistance, float fadestart);
    void UpdateTransparency(int whichx, int whichy, const Vector3& viewer, float viewdistance, float fadestart);
    void UpdateTransparencyother(int whichx, int whichy);
    void UpdateTransparencyotherother(int whichx, int whichy, const Vector3& viewer, float viewdistance, float fadestart);
};

#endif
