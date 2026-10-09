// Unit tests for the App layer.
//
// References AppLib and Catch2 only.
//
// The first tests in the project that load a real skeleton and tick real
// simulation code. They became possible when Skeleton::Load stopped driving the
// fixed-function matrix stack for its arithmetic: a figure and its models can
// now be read straight out of Data/ with no GL context and no audio device.

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

#include "Animation/Skeleton.hpp"
#include "Environment/Terrain.hpp"
#include "GameAssets.hpp"
#include "GameState.hpp"
#include "Objects/PersonType.hpp"

namespace
{

// Data/ is resolved relative to the working directory Catch2 runs the tests in.
bool existsInDataFolder(const std::string& path)
{
	FILE* file = fopen(path.c_str(), "rb");
	if (!file) {
		return false;
	}
	fclose(file);
	return true;
}

const float kDegreesToRadians = 0.017453292519943295f;

// Post-multiply the running matrix by operand.
void multiplyInto(float matrix[16], const float operand[16])
{
	float product[16] = { 0 };
	for (int column = 0; column < 4; column++) {
		for (int row = 0; row < 4; row++) {
			float sum = 0;
			for (int k = 0; k < 4; k++) {
				sum += matrix[k * 4 + row] * operand[column * 4 + k];
			}
			product[column * 4 + row] = sum;
		}
	}
	for (int i = 0; i < 16; i++) {
		matrix[i] = product[i];
	}
}

// What each glRotatef put on the stack.
void appendRotation(float matrix[16], float angle, float x, float y, float z)
{
	const float cosine = static_cast<float>(cos(angle * kDegreesToRadians));
	const float sine = static_cast<float>(sin(angle * kDegreesToRadians));
	const float oneMinusCosine = 1.0f - cosine;

	float rotation[16] = { 0 };
	rotation[0] = x * x * oneMinusCosine + cosine;
	rotation[1] = y * x * oneMinusCosine + z * sine;
	rotation[2] = z * x * oneMinusCosine - y * sine;
	rotation[4] = x * y * oneMinusCosine - z * sine;
	rotation[5] = y * y * oneMinusCosine + cosine;
	rotation[6] = z * y * oneMinusCosine + x * sine;
	rotation[8] = x * z * oneMinusCosine + y * sine;
	rotation[9] = y * z * oneMinusCosine - x * sine;
	rotation[10] = z * z * oneMinusCosine + cosine;
	rotation[15] = 1;

	multiplyInto(matrix, rotation);
}

// What each glTranslatef put on the stack.
void appendTranslation(float matrix[16], const Vector3& point)
{
	float translation[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	translation[12] = point.x;
	translation[13] = point.y;
	translation[14] = point.z;

	multiplyInto(matrix, translation);
}

// Where a vertex ends up, by rebuilding the matrix the stack ended up holding
// and reading its translation column back, rather than by rotating the point
// three times over.
Vector3 boneSpaceVertex(const Vector3& point, const Muscle& muscle)
{
	float matrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	appendRotation(matrix, muscle.rotate3, 0, 1, 0);
	appendRotation(matrix, muscle.rotate2 - 90, 0, 0, 1);
	appendRotation(matrix, muscle.rotate1 - 90, 0, 1, 0);
	appendTranslation(matrix, point);
	return Vector3(matrix[12], matrix[13], matrix[14]);
}

std::unique_ptr<Skeleton> loadWolfSkeleton()
{
	static GameState gamestate;
	static GameAssets assets;
	static bool typesLoaded = false;
	if (!typesLoaded) {
		PersonType::Load(assets.graphics);
		typesLoaded = true;
	}

	const PersonType& wolf = assets.graphics.types[wolftype];
	auto skeleton = std::make_unique<Skeleton>();
	// Not the tutorial and not clothed: the wolf ships no clothes figure, and the
	// tutorial branch rewrites texture coordinates instead of loading them.
	skeleton->Load(
		wolf.figureFileName,
		wolf.lowFigureFileName,
		wolf.clothesFileName,
		wolf.modelFileNames[0],
		wolf.modelFileNames[1],
		wolf.modelFileNames[2],
		wolf.modelFileNames[3],
		wolf.modelFileNames[4],
		wolf.modelFileNames[5],
		wolf.modelFileNames[6],
		wolf.lowModelFileName,
		wolf.modelClothesFileName,
		wolf.clothes,
		false,
		[]() {});
	return skeleton;
}

bool jointsAreFinite(const Skeleton& skeleton)
{
	for (const Joint& joint : skeleton.joints) {
		const float numbers[] = { joint.position.x, joint.position.y, joint.position.z,
			joint.velocity.x, joint.velocity.y, joint.velocity.z };
		for (float number : numbers) {
			if (!std::isfinite(number)) {
				return false;
			}
		}
	}
	return true;
}

float downwardVelocity(const Skeleton& skeleton)
{
	float total = 0;
	for (const Joint& joint : skeleton.joints) {
		total += joint.velocity.y;
	}
	return total;
}

} // namespace

TEST_CASE("a skeleton loads and simulates without a GL context", "[skeleton][simulation]")
{
	REQUIRE(existsInDataFolder("Data/Skeleton/BasicFigureWolf"));
	REQUIRE(existsInDataFolder("Data/Models/Wolf.solid"));

	const auto skeleton = loadWolfSkeleton();

	SECTION("the shipped figure produces a usable skeleton")
	{
		REQUIRE(skeleton->joints.size() > 0);
		REQUIRE(skeleton->muscles.size() > 0);
		REQUIRE(skeleton->model[0].vertexNum > 0);
		REQUIRE(jointsAreFinite(*skeleton));
	}

	SECTION("every muscle spans two joints the skeleton actually has")
	{
		for (const Muscle& muscle : skeleton->muscles) {
			REQUIRE(muscle.parent1 != nullptr);
			REQUIRE(muscle.parent2 != nullptr);
		}
	}

	SECTION("gravity pulls the joints downward")
	{
		const float before = downwardVelocity(*skeleton);
		// The amount GameState starts a session with. The up axis is +Y, so this
		// is the value that actually falls.
		const float gravity = -10;
		float scale = 1;
		for (int frame = 0; frame < 10; frame++) {
			skeleton->DoGravity(&scale, 1, gravity);
		}
		REQUIRE(downwardVelocity(*skeleton) < before);
	}

	SECTION("bone connect constraints keep the skeleton real over many frames")
	{
		std::vector<Vector3> velocitiesBefore;
		for (const Joint& joint : skeleton->joints) {
			velocitiesBefore.push_back(joint.velocity);
		}

		// A Terrain is a couple of megabytes of fixed arrays, so it goes on the
		// heap - one on the stack overflows the thread the suite runs on.
		const auto terrain = std::make_unique<Terrain>();
		GameState gamestate;
		Vector3 coords(0, 10, 0);
		float scale = 1;
		int jointstartarray[26] = {};
		int jointendarray[26] = {};

		for (int frame = 0; frame < 200; frame++) {
			skeleton->DoConstraints(&coords, &scale, false, gamestate.bloodtoggle, 0.01f,
				*terrain, gamestate.environment, gamestate.camerashake, gamestate.freeze,
				gamestate.detail, jointstartarray, jointendarray);
		}

		REQUIRE(jointsAreFinite(*skeleton));

		bool anyVelocityMoved = false;
		for (unsigned joint = 0; joint < skeleton->joints.size(); joint++) {
			if (findDistance(&skeleton->joints[joint].velocity, &velocitiesBefore[joint]) > 0) {
				anyVelocityMoved = true;
			}
		}
		// If this ever stops holding, the constraint pass has stopped running at
		// all, which is the failure a headless run would otherwise hide.
		REQUIRE(anyVelocityMoved);
	}
}

TEST_CASE("a loaded skeleton puts its vertices in the bone space its muscles define", "[skeleton]")
{
	REQUIRE(existsInDataFolder("Data/Models/Wolf.solid"));

	GameState gamestate;
	GameAssets assets;
	PersonType::Load(assets.graphics);
	const PersonType& wolf = assets.graphics.types[wolftype];

	// A model on its own, before Skeleton::Load moves its vertices into bone
	// space, so the transform being tested can be recomputed from scratch.
	Model reference;
	REQUIRE(reference.loadnotex(wolf.modelFileNames[0]));
	reference.Rotate(180, 0, 0);
	reference.Scale(.04f, .04f, .04f);

	const auto skeleton = loadWolfSkeleton();
	REQUIRE(skeleton->model[0].vertexNum == reference.vertexNum);

	int verticesInTheWrongPlace = 0;
	for (short vertex = 0; vertex < reference.vertexNum; vertex++) {
		const Muscle& muscle = skeleton->muscles[skeleton->model[0].owner[vertex]];
		const Vector3 expected = boneSpaceVertex(
			reference.vertex[vertex] - (muscle.parent1->position + muscle.parent2->position) / 2,
			muscle);
		if (findDistance(&expected, &skeleton->model[0].vertex[vertex]) > 0.0001f) {
			verticesInTheWrongPlace++;
		}
	}
	// Every vertex is a rigid rotation of where the raw model had it, so a
	// change to the three rotations, their order, or their axes moves it.
	REQUIRE(verticesInTheWrongPlace == 0);
}
