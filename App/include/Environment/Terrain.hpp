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

    std::unique_ptr<std::vector<unsigned int>[]> patchobjects_storage;
    std::vector<unsigned int> (*patchobjects)[subdivision] = nullptr;

    std::unique_ptr<float[]> heightmap_storage;
    float (*heightmap)[max_terrain_size + 1] = nullptr;

    std::unique_ptr<Vector3[]> normals_storage;
    Vector3 (*normals)[max_terrain_size] = nullptr;

    std::unique_ptr<Vector3[]> facenormals_storage;
    Vector3 (*facenormals)[max_terrain_size] = nullptr;

    std::unique_ptr<Vector3[]> triangles_storage;
    Vector3 (*triangles)[3] = nullptr;

    std::unique_ptr<float[]> colors_storage;
    float (*colors)[max_terrain_size][4] = nullptr;

    std::unique_ptr<float[]> opacityother_storage;
    float (*opacityother)[max_terrain_size] = nullptr;

    std::unique_ptr<float[]> texoffsetx_storage;
    float (*texoffsetx)[max_terrain_size] = nullptr;

    std::unique_ptr<float[]> texoffsety_storage;
    float (*texoffsety)[max_terrain_size] = nullptr;

    std::unique_ptr<int[]> numtris_storage;
    int (*numtris)[subdivision] = nullptr;

    std::unique_ptr<int[]> textureness_storage;
    int (*textureness)[subdivision] = nullptr;

    std::unique_ptr<bool[]> visible_storage;
    bool (*visible)[subdivision] = nullptr;

    std::unique_ptr<float[]> avgypatch_storage;
    float (*avgypatch)[subdivision] = nullptr;

    std::unique_ptr<float[]> maxypatch_storage;
    float (*maxypatch)[subdivision] = nullptr;

    std::unique_ptr<float[]> minypatch_storage;
    float (*minypatch)[subdivision] = nullptr;

    std::unique_ptr<float[]> heightypatch_storage;
    float (*heightypatch)[subdivision] = nullptr;

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
