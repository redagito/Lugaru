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

#ifndef _MODELS_HPP_
#define _MODELS_HPP_

// Graphics
#include "Graphic/Decal.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"

// Foundation
#include "Math/Vector3.hpp"
#include "Utils/binio.h"
#include "Utils/Callbacks.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

//
// Textures List
//
struct ModelTexture
{
    long xsz, ysz;
    GLubyte* txt;
};

//
// Model Structures
//

class TexturedTriangle
{
public:
    // A triangle has exactly three vertices, so these are std::array rather than
    // a C array: the index is nearly always a literal 0, 1 or 2, but the values
    // are indices into Model::vertex, which comes straight out of a model file.
    std::array<short, 3> vertex;
    std::array<float, 3> gx;
    std::array<float, 3> gy;
    Vector3 facenormal;
};

#define max_model_decals 300

enum ModelType
{
    nothing = 0,
    notextype = 1,
    rawtype = 2,
    decalstype = 3,
    normaltype = 4
};

class Model
{
public:
    short vertexNum;

    ModelType type;

    // Per-vertex data, sized by vertexNum at load. std::vector rather than raw
    // malloc'd pointers so an index that runs past the end throws: these are
    // indexed by vertex IDs read out of the model file, which is untrusted input,
    // and by TexturedTriangle::vertex, which indexes into this same array.
    std::vector<int> owner;
    std::vector<Vector3> vertex;
    std::vector<Vector3> normals;
    std::vector<TexturedTriangle> Triangles;
    std::vector<GLfloat> vArray;

    /*
    int owner[max_textured_triangle];
    Vector3 vertex[max_model_vertex];
    Vector3 normals[max_model_vertex];
    GLfloat vArray[max_textured_triangle*24];*/

    Texture textureptr;
    ModelTexture modelTexture;
    bool color;

    Vector3 boundingspherecenter;
    float boundingsphereradius;

    std::vector<Decal> decals;

    bool flat;

    Model();
    ~Model();
    void MakeDecal(decal_type atype, Vector3* where, float* size, float* opacity, float* rotation);
    void MakeDecal(decal_type atype, Vector3 where, float size, float opacity, float rotation);

    const Vector3& getTriangleVertex(unsigned triangleId, unsigned vertexId) const;
    void drawdecals(Texture shadowtexture, Texture bloodtexture, Texture bloodtexture2, Texture breaktexture, bool toggledecals, float multiplier);
    int SphereCheck(Vector3* p1, float radius, Vector3* p, Vector3* move, float* rotate);
    int SphereCheckPossible(Vector3* p1, float radius, Vector3* move, float* rotate);
    int LineCheck(Vector3* p1, Vector3* p2, Vector3* p, Vector3* move, float* rotate);
    int LineCheckPossible(Vector3* p1, Vector3* p2, Vector3* p, Vector3* move, float* rotate);
    int LineCheckSlidePossible(Vector3* p1, Vector3* p2, Vector3* move, float* rotate);
    void UpdateVertexArray();
    void UpdateVertexArrayNoTex();
    void UpdateVertexArrayNoTexNoNorm();
    bool loadnotex(const std::string& filename);
    bool loadraw(const std::string& filename);
    bool load(const std::string& filename, ProgressCallback callback);
    bool loaddecal(const std::string& filename);
    void Scale(float xscale, float yscale, float zscale);
    void FlipTexCoords();
    void UniformTexCoords();
    void ScaleTexCoords(float howmuch);
    void ScaleNormals(float xscale, float yscale, float zscale);
    void Translate(float xtrans, float ytrans, float ztrans);
    void CalculateNormals(bool facenormalise, ProgressCallback callback);
    void draw();
    void drawdifftex(Texture texture);
    void drawimmediate();
    void Rotate(float xang, float yang, float zang);
    void deleteDeadDecals();

private:
    void DeleteDecal(int which);
    void deallocate();
    /* indices of triangles that might collide */
    std::vector<unsigned int> possible;
};

#endif
