#pragma once

#include <cmath>

constexpr float PI = 3.14159265358979323846f;

static inline float toRadians(float angle)
{
	return static_cast<float>(PI / 180.0F * angle);
}