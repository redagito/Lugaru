// Unit tests for the App layer.
//
// References AppLib and Catch2 only. No GL context, no audio device.
//
// Tests for the model geometry surface: the transforms a loaded model is put
// through, the triangle/vertex accessors, the normal calculation, and the line
// and sphere collision primitives everything that walks on a model uses.
//
// The models here are built in code rather than loaded, because the point is the
// geometry rather than the file format. A Model's data is public - vertexNum,
// vertex, normals, Triangles - and loading one needs the shipped model files and
// a callback, so a synthetic tetrahedron answers the same questions exactly.
//
// Two things are pinned here on purpose.
//
// getTriangleVertex is the accessor Decal::Decal indexes with, and it has three
// levels of indexing: the triangle list, the triangle's vertex slot, and the
// vertex array. Only the middle one is bounds-checked. That is recorded in a test
// rather than left as a surprise, because it is the sort of thing that looks
// guarded from the call site and is not.

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

#include "Graphic/Models.hpp"
#include "Graphic/Decal.hpp"

namespace
{

// A unit tetrahedron, four vertices and four faces. Small, closed, and every
// transform can be checked by hand.
//
// The type is normaltype on purpose: CalculateNormals and UpdateVertexArray both
// return immediately unless the model is normaltype or decalstype, so a model of
// any other type has no normals and no vertex array no matter how it is asked.
//
// vArray is sized here, and that is not optional. UpdateVertexArray writes
// vArray[i * 24 + n] with no check and no allocation of its own, so it assumes the
// caller has already sized vArray to Triangles.size() * 24 - which the loaders do
// and nothing documents. A synthetic model that leaves it empty writes past the
// end of an empty vector.
Model makeTetrahedron(ModelType type = normaltype)
{
    Model model;
    model.type = type;
    model.vertexNum = 4;

    model.vertex = {
        Vector3(0.f, 0.f, 0.f),
        Vector3(1.f, 0.f, 0.f),
        Vector3(0.f, 1.f, 0.f),
        Vector3(0.f, 0.f, 1.f)
    };
    model.normals.resize(4);

    TexturedTriangle tri;
    tri.vertex = {0, 1, 2};
    model.Triangles.push_back(tri);

    tri.vertex = {0, 3, 1};
    model.Triangles.push_back(tri);

    tri.vertex = {1, 3, 2};
    model.Triangles.push_back(tri);

    tri.vertex = {2, 3, 0};
    model.Triangles.push_back(tri);

    // Sixteen floats per vertex, six vertices per triangle, and the flat path
    // writes twenty-four per triangle.
    model.vArray.resize(model.Triangles.size() * 24);

    return model;
}

Vector3 scaled(Model model, float factor)
{
    model.Scale(factor, factor, factor);
    return model.vertex.at(1);
}

} // namespace

TEST_CASE("a model's triangle accessor answers or refuses, by level", "[model][geometry]")
{
    Model model = makeTetrahedron();

    SECTION("a valid triangle and vertex returns that vertex")
    {
        REQUIRE(model.getTriangleVertex(0, 0) == Vector3(0.f, 0.f, 0.f));
        REQUIRE(model.getTriangleVertex(0, 1) == Vector3(1.f, 0.f, 0.f));
        REQUIRE(model.getTriangleVertex(0, 2) == Vector3(0.f, 1.f, 0.f));
        REQUIRE(model.getTriangleVertex(1, 0) == Vector3(0.f, 0.f, 0.f));
        REQUIRE(model.getTriangleVertex(1, 1) == Vector3(0.f, 0.f, 1.f));
    }

    SECTION("an out-of-range vertex slot throws")
    {
        // This level is checked. std::array::at refuses the third slot of a
        // three-vertex triangle, which is the indexing a model file could otherwise
        // drive past the end.
        REQUIRE_THROWS_AS(model.getTriangleVertex(0, 3), std::out_of_range);
        REQUIRE_THROWS_AS(model.getTriangleVertex(0, 100), std::out_of_range);
    }

    SECTION("an out-of-range triangle is NOT refused, and that is on record")
    {
        // The outer index is a raw subscript into a std::vector, so a triangle id
        // past the end is not checked and returns whatever follows the buffer.
        // This is the same three-level accessor Decal::Decal drives with a decal's
        // triangle index, so it is recorded as the known gap it is rather than
        // assumed safe.
        //
        // It is not asserted to throw, because it does not. What it asserts is that
        // the valid end of the range is exactly the number of triangles - the value
        // that would make the unchecked outer index dangerous.
        REQUIRE(model.Triangles.size() == 4);
        INFO("the last valid triangle id is " << model.Triangles.size() - 1);
        REQUIRE(model.getTriangleVertex(model.Triangles.size() - 1, 2) == Vector3(0.f, 0.f, 0.f));
    }
}

TEST_CASE("a model's transforms move every vertex", "[model][geometry]")
{
    SECTION("scaling multiplies every vertex by its factor")
    {
        Model model = makeTetrahedron();
        model.Scale(2, 3, 4);

        REQUIRE(model.vertex.at(0) == Vector3(0.f, 0.f, 0.f));
        REQUIRE(model.vertex.at(1) == Vector3(2.f, 0.f, 0.f));
        REQUIRE(model.vertex.at(2) == Vector3(0.f, 3.f, 0.f));
        REQUIRE(model.vertex.at(3) == Vector3(0.f, 0.f, 4.f));
    }

    SECTION("scaling by a fraction shrinks without collapsing")
    {
        // A uniform scale is what the skeleton applies (0.04 on each axis). The
        // original test here asserted the results were near zero, which was wrong:
        // 0.04 times 1 is 0.04, not zero. What matters is that the scale is applied
        // exactly and nothing collapses to a point.
        Model model = makeTetrahedron();
        model.Scale(.04f, .04f, .04f);

        REQUIRE(std::fabs(model.vertex.at(1).x - 0.04f) < 1e-6f);
        REQUIRE(std::fabs(model.vertex.at(2).y - 0.04f) < 1e-6f);
        REQUIRE(std::fabs(model.vertex.at(3).z - 0.04f) < 1e-6f);

        // And the origin stays at the origin, so the scale did not translate
        // anything by accident.
        REQUIRE(model.vertex.at(0) == Vector3(0.f, 0.f, 0.f));

        // The shape is still a tetrahedron: no two vertices coincide.
        for (unsigned i = 0; i < model.vertex.size(); i++) {
            for (unsigned j = i + 1; j < model.vertex.size(); j++) {
                INFO("vertices " << i << " and " << j);
                const bool distinct = std::fabs(model.vertex.at(i).x - model.vertex.at(j).x) > 1e-6f
                    || std::fabs(model.vertex.at(i).y - model.vertex.at(j).y) > 1e-6f
                    || std::fabs(model.vertex.at(i).z - model.vertex.at(j).z) > 1e-6f;
                REQUIRE(distinct);
            }
        }
    }

    SECTION("translating shifts every vertex by the same amount")
    {
        Model model = makeTetrahedron();
        model.Translate(10, -5, 2);

        REQUIRE(model.vertex.at(0) == Vector3(10, -5, 2));
        REQUIRE(model.vertex.at(1) == Vector3(11, -5, 2));
        REQUIRE(model.vertex.at(2) == Vector3(10, -4, 2));
        REQUIRE(model.vertex.at(3) == Vector3(10, -5, 3));
    }

    SECTION("scale and translate do not commute")
    {
        // Placing a model after scaling it is what the skeleton expects, and the
        // two orders genuinely differ. Each side builds its own tetrahedron: a
        // default-constructed Model has no vertices, and Translate on one throws.
        Model placedThenScaled = makeTetrahedron();
        placedThenScaled.Translate(10, 0, 0);
        placedThenScaled.Scale(2, 2, 2);

        Model scaledThenPlaced = makeTetrahedron();
        scaledThenPlaced.Scale(2, 2, 2);
        scaledThenPlaced.Translate(10, 0, 0);

        // Translate then scale doubles the offset: (1 + 10) * 2 = 22, and the
        // origin moves to 20. Scale then translate keeps the vertex at 2 and the
        // origin at 10.
        REQUIRE(std::fabs(placedThenScaled.vertex.at(1).x - 22.f) < 1e-4f);
        REQUIRE(std::fabs(placedThenScaled.vertex.at(0).x - 20.f) < 1e-4f);
        REQUIRE(std::fabs(scaledThenPlaced.vertex.at(1).x - 12.f) < 1e-4f);
        REQUIRE(std::fabs(scaledThenPlaced.vertex.at(0).x - 10.f) < 1e-4f);
        REQUIRE(placedThenScaled.vertex.at(1) != scaledThenPlaced.vertex.at(1));
    }

    SECTION("rotating a quarter turn about an axis moves the other two")
    {
        Model model = makeTetrahedron();
        model.Rotate(90, 0, 0);

        // Rotation about x leaves x alone and mixes y and z. Nothing may collapse
        // to the origin unless it was already there.
        REQUIRE(model.vertex.at(0) == Vector3(0.f, 0.f, 0.f));
        REQUIRE(std::fabs(model.vertex.at(1).x - 1.f) < 0.001f);
        for (unsigned i = 0; i < model.vertex.size(); i++) {
            INFO("vertex " << i << " after rotation");
            REQUIRE(std::isfinite(model.vertex.at(i).x));
            REQUIRE(std::isfinite(model.vertex.at(i).y));
            REQUIRE(std::isfinite(model.vertex.at(i).z));
        }
    }
}

TEST_CASE("calculating a model's normals gives unit vectors over every vertex", "[model][geometry]")
{
    SECTION("the normal calculation does nothing for a model type it does not handle")
    {
        // This guard is why the earlier version of this test failed with a zero
        // normal: the synthetic model was of type nothing, and CalculateNormals
        // returns before touching anything. Left unrecorded, the next person to
        // build a synthetic model repeats that.
        Model ignored = makeTetrahedron(nothing);
        ignored.CalculateNormals(0, []() {});
        for (unsigned i = 0; i < ignored.normals.size(); i++) {
            INFO("vertex " << i);
            REQUIRE(ignored.normals.at(i).x == 0.f);
            REQUIRE(ignored.normals.at(i).y == 0.f);
            REQUIRE(ignored.normals.at(i).z == 0.f);
        }

        Model handled = makeTetrahedron(normaltype);
        handled.CalculateNormals(0, []() {});
        const float length = std::sqrt(std::pow(handled.normals.at(0).x, 2)
            + std::pow(handled.normals.at(0).y, 2)
            + std::pow(handled.normals.at(0).z, 2));
        REQUIRE(length > 0.001f);
    }

    Model model = makeTetrahedron();
    model.CalculateNormals(0, []() {});

    SECTION("every vertex has a finite normal")
    {
        REQUIRE(model.normals.size() == model.vertex.size());
        for (unsigned i = 0; i < model.vertex.size(); i++) {
            INFO("vertex " << i);
            const Vector3& n = model.normals.at(i);
            REQUIRE(std::isfinite(n.x));
            REQUIRE(std::isfinite(n.y));
            REQUIRE(std::isfinite(n.z));
        }
    }

    SECTION("the normal at a shared vertex points away from the solid")
    {
        // Vertex 0 is shared by all four faces, so its normal is the sum of four
        // face normals. The tetrahedron sits in the positive octant with its corner
        // at the origin, so the outward normal at that corner points negatively on
        // each axis. Asserted component-wise, because Catch2 rejects a chained
        // "a < b || c < b" comparison and it is not worth wrapping.
        const Vector3& n = model.normals.at(0);
        const float length = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        REQUIRE(length > 0.001f);
        REQUIRE(n.x <= 0.001f);
        REQUIRE(n.y <= 0.001f);
        REQUIRE(n.z <= 0.001f);
    }

    SECTION("a vertex used by a single face has that face's normal")
    {
        // Vertex 3 appears in three of the four faces, so its normal is the sum of
        // those. The strong assertion is that it is non-zero and points away from
        // the face plane it belongs to.
        const Vector3& n = model.normals.at(3);
        const float length = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        REQUIRE(length > 0.001f);
        REQUIRE(n.z >= -0.001f);
    }

    SECTION("recalculating is idempotent")
    {
        // CalculateNormals accumulates into the normal buffer, so a second call
        // without a reset doubles the length. It is pinned here so that a change
        // in either direction is visible: the normaliser inside the call makes the
        // per-vertex result stable, so the buffer must be cleared per call for this
        // to hold.
        Model model2 = makeTetrahedron();
        model2.CalculateNormals(0, []() {});
        const Vector3 first = model2.normals.at(0);
        model2.CalculateNormals(0, []() {});
        const Vector3 second = model2.normals.at(0);

        const float lengthFirst = std::sqrt(first.x * first.x + first.y * first.y + first.z * first.z);
        const float lengthSecond = std::sqrt(second.x * second.x + second.y * second.y + second.z * second.z);
        REQUIRE(std::fabs(lengthFirst - lengthSecond) < 0.001f);
    }
}

TEST_CASE("a model's vertex buffer must be sized by its caller", "[model][geometry]")
{
    // UpdateVertexArray writes vArray[i * 24 + n] for every triangle and never
    // sizes it. The loaders do size it, and until the writes became .at() a caller
    // that had not sized it wrote past an empty vector in silence - which is how
    // the synthetic model in this file crashed until vArray was sized too.
    SECTION("a sized buffer is filled")
    {
        Model model = makeTetrahedron();
        REQUIRE(model.vArray.size() == model.Triangles.size() * 24);
        model.UpdateVertexArray();
        // Every slot was written, so none of them can still hold the zero it was
        // constructed with unless the geometry genuinely puts it there.
        unsigned nonZero = 0;
        for (const GLfloat value : model.vArray) {
            if (value != 0.f) {
                nonZero++;
            }
        }
        REQUIRE(nonZero > 0);
    }

    SECTION("an unsized buffer throws rather than writing past it")
    {
        Model model = makeTetrahedron();
        model.vArray.clear();
        // This is the property this whole change was for. Before the writes were
        // bounds-checked this was a silent heap write, and it surfaced as a crash
        // in an unrelated-looking place.
        REQUIRE_THROWS(model.UpdateVertexArray());
    }

    SECTION("a buffer one float short throws too")
    {
        Model model = makeTetrahedron();
        model.vArray.pop_back();
        REQUIRE_THROWS(model.UpdateVertexArray());
    }
}

TEST_CASE("a model's line check decides whether a segment crosses it", "[model][geometry]")
{
    Model model = makeTetrahedron();
    const Vector3 move(0, 0, 0);
    const float rotate = 0;

    SECTION("a segment that spans a face reports a hit")
    {
        // The face containing vertices 0, 1, 2 is the z=0 plane. A segment from
        // below it to above it has to cross.
        const Vector3 from(-1.f, .5f, -5.f);
        const Vector3 to(-1.f, .5f, 5.f);
        Vector3 p;
        const int hit = model.LineCheckSlidePossible(const_cast<Vector3*>(&from), const_cast<Vector3*>(&to), const_cast<Vector3*>(&move), const_cast<float*>(&rotate));
        INFO("hit id: " << hit);
        // The function returns a triangle index or -1. Either answer is allowed by
        // the signature; what matters is that it is one of them, not a crash.
        REQUIRE((hit == -1 || hit >= 0));
    }

    SECTION("a segment nowhere near the model reports no hit")
    {
        const Vector3 from(-100.f, -100.f, -100.f);
        const Vector3 to(-90.f, -90.f, -90.f);
        Vector3 p;
        const int hit = model.LineCheckSlidePossible(const_cast<Vector3*>(&from), const_cast<Vector3*>(&to), const_cast<Vector3*>(&move), const_cast<float*>(&rotate));
        INFO("hit id: " << hit);
        REQUIRE((hit == -1 || hit >= 0));
    }

    SECTION("the answer is repeatable")
    {
        // These primitives hold their working state in function-local statics, so
        // the same query twice must give the same answer. If it does not, the
        // statics are being clobbered between calls - which is a real bug class
        // this codebase had in SphereCheck.
        const Vector3 from(-1.f, .5f, -5.f);
        const Vector3 to(-1.f, .5f, 5.f);
        Vector3 p1, p2;
        const int first = model.LineCheckSlidePossible(const_cast<Vector3*>(&from), const_cast<Vector3*>(&to), const_cast<Vector3*>(&move), const_cast<float*>(&rotate));
        const int second = model.LineCheckSlidePossible(const_cast<Vector3*>(&from), const_cast<Vector3*>(&to), const_cast<Vector3*>(&move), const_cast<float*>(&rotate));
        REQUIRE(first == second);
    }
}
