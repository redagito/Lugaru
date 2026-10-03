#pragma once

#include "Math/Vector3.hpp"

#include <numbers>

// utility functions

// TODO: this is slightly incorrect
inline float roughDirection(Vector3 vec)
{
	Normalise(&vec);
	float angle = -asin(-vec.x) * 180 / std::numbers::pi_v<float>;
	if (vec.z < 0) {
		angle = 180 - angle;
	}
	return angle;
}

inline float roughDirectionTo(Vector3 start, Vector3 end)
{
	return roughDirection(end - start);
}

inline float pitchOf(Vector3 vec)
{
	Normalise(&vec);
	return -asin(vec.y) * 180 / std::numbers::pi_v<float>;
}

inline float pitchTo(Vector3 start, Vector3 end)
{
	return pitchOf(end - start);
}

inline float sq(float n)
{
	return n * n;
}

inline float stepTowardf(float from, float to, float by)
{
	if (fabs(from - to) < by) {
		return to;
	}
	else if (from > to) {
		return from - by;
	}
	else {
		return from + by;
	}
}