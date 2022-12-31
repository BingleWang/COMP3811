#ifndef MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
#define MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA

#include <cmath>
#include <cassert>
#include <cstdlib>

#include "vec3.hpp"
#include "vec4.hpp"
#include "utils.hpp"

/** Mat44f: 4x4 matrix with floats
 *
 * See vec2f.hpp for discussion. Similar to the implementation, the Mat44f is
 * intentionally kept simple and somewhat bare bones.
 *
 * The matrix is stored in row-major order (careful when passing it to OpenGL).
 *
 * The overloaded operator () allows access to individual elements. Example:
 *    Mat44f m = ...;
 *    float m12 = m(1,2);
 *    m(0,3) = 3.f;
 *
 * The matrix is arranged as:
 *
 *   ⎛ 0,0  0,1  0,2  0,3 ⎞
 *   ⎜ 1,0  1,1  1,2  1,3 ⎟
 *   ⎜ 2,0  2,1  2,2  2,3 ⎟
 *   ⎝ 3,0  3,1  3,2  3,3 ⎠
 */
struct Mat44f
{
	float v[16];

	constexpr
	float& operator() (std::size_t aI, std::size_t aJ) noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
	constexpr
	float const& operator() (std::size_t aI, std::size_t aJ) const noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
};

// Identity matrix
constexpr Mat44f kIdentity44f = { {
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
} };

// Common operators for Mat44f.
// Note that you will need to implement these yourself.

constexpr
Mat44f operator*( Mat44f const& aLeft, Mat44f const& aRight ) noexcept
{
	//TODO: your implementation goes here
	Vec4f leftRow0 = { aLeft(0, 0), aLeft(0, 1),  aLeft(0, 2),  aLeft(0, 3) };
	Vec4f leftRow1 = { aLeft(1, 0), aLeft(1, 1),  aLeft(1, 2),  aLeft(1, 3) };
	Vec4f leftRow2 = { aLeft(2, 0), aLeft(2, 1),  aLeft(2, 2),  aLeft(2, 3) };
	Vec4f leftRow3 = { aLeft(3, 0), aLeft(3, 1),  aLeft(3, 2),  aLeft(3, 3) };
	Vec4f rightColumn0 = { aRight(0, 0), aRight(1, 0), aRight(2, 0), aRight(3, 0) };
	Vec4f rightColumn1 = { aRight(0, 1), aRight(1, 1), aRight(2, 1), aRight(3, 1) };
	Vec4f rightColumn2 = { aRight(0, 2), aRight(1, 2), aRight(2, 2), aRight(3, 2) };
	Vec4f rightColumn3 = { aRight(0, 3), aRight(1, 3), aRight(2, 3), aRight(3, 3) };

	auto result = kIdentity44f;

	result(0, 0) = dot(leftRow0, rightColumn0);
	result(0, 1) = dot(leftRow0, rightColumn1);
	result(0, 2) = dot(leftRow0, rightColumn2);
	result(0, 3) = dot(leftRow0, rightColumn3);

	result(1, 0) = dot(leftRow1, rightColumn0);
	result(1, 1) = dot(leftRow1, rightColumn1);
	result(1, 2) = dot(leftRow1, rightColumn2);
	result(1, 3) = dot(leftRow1, rightColumn3);

	result(2, 0) = dot(leftRow2, rightColumn0);
	result(2, 1) = dot(leftRow2, rightColumn1);
	result(2, 2) = dot(leftRow2, rightColumn2);
	result(2, 3) = dot(leftRow2, rightColumn3);

	result(3, 0) = dot(leftRow3, rightColumn0);
	result(3, 1) = dot(leftRow3, rightColumn1);
	result(3, 2) = dot(leftRow3, rightColumn2);
	result(3, 3) = dot(leftRow3, rightColumn3);

	return result;
}

constexpr
Vec4f operator*( Mat44f const& aLeft, Vec4f const& aRight ) noexcept
{
	//TODO: your implementation goes here
	Vec4f leftRow0 = { aLeft(0, 0), aLeft(0, 1),  aLeft(0, 2),  aLeft(0, 3) };
	Vec4f leftRow1 = { aLeft(1, 0), aLeft(1, 1),  aLeft(1, 2),  aLeft(1, 3) };
	Vec4f leftRow2 = { aLeft(2, 0), aLeft(2, 1),  aLeft(2, 2),  aLeft(2, 3) };
	Vec4f leftRow3 = { aLeft(3, 0), aLeft(3, 1),  aLeft(3, 2),  aLeft(3, 3) };

	auto result = aRight;
	result.x = dot(leftRow0, aRight);
	result.y = dot(leftRow1, aRight);
	result.z = dot(leftRow2, aRight);
	result.w = dot(leftRow3, aRight);

	return result;
}

// Functions:

inline
Mat44f make_rotation_x( float aAngle ) noexcept
{
	//TODO: your implementation goes here
	Mat44f result = kIdentity44f;

	result(0, 0) = 1.0f;
	result(0, 1) = 0.0f;
	result(0, 2) = 0.0f;
	result(0, 3) = 0.0f;

	result(1, 0) = 0.0f;
	result(1, 1) = std::cos(toRadians(aAngle));
	result(1, 2) = -std::sin(toRadians(aAngle));
	result(1, 3) = 0.0f;

	result(2, 0) = 0.0f;
	result(2, 1) = std::sin(toRadians(aAngle));
	result(2, 2) = std::cos(toRadians(aAngle));
	result(2, 3) = 0.0f;

	result(3, 0) = 0.0f;
	result(3, 1) = 0.0f;
	result(3, 2) = 0.0f;
	result(3, 3) = 1.0f;

	return result;
}

inline
Mat44f make_rotation_y( float aAngle ) noexcept
{
	//TODO: your implementation goes here
	Mat44f result;

	result(0, 0) = std::cos(toRadians(aAngle));
	result(0, 1) = 0.0f;
	result(0, 2) = std::sin(toRadians(aAngle));
	result(0, 3) = 0.0f;

	result(1, 0) = 0.0f;
	result(1, 1) = 1.0f;
	result(1, 2) = 0.0f;
	result(1, 3) = 0.0f;

	result(2, 0) = -std::sin(toRadians(aAngle));
	result(2, 1) = 0.0f;
	result(2, 2) = std::sin(toRadians(aAngle));
	result(2, 3) = std::cos(toRadians(aAngle));

	result(3, 0) = 0.0f;
	result(3, 1) = 0.0f;
	result(3, 2) = 0.0f;
	result(3, 3) = 1.0f;

	return result;
}

inline
Mat44f make_rotation_z( float aAngle ) noexcept
{
	//TODO: your implementation goes here
	Mat44f result = kIdentity44f;

	result(0, 0) = std::cos(toRadians(aAngle));
	result(0, 1) = -std::sin(toRadians(aAngle));
	result(0, 2) = 0.0f;
	result(0, 3) = 0.0f;

	result(1, 0) = std::sin(toRadians(aAngle));
	result(1, 1) = std::cos(toRadians(aAngle));
	result(1, 2) = 0.0f;
	result(1, 3) = 0.0f;

	result(2, 0) = 0.0f;
	result(2, 1) = 0.0f;
	result(2, 2) = 1.0f;
	result(2, 3) = 0.0f;

	result(3, 0) = 0.0f;
	result(3, 1) = 0.0f;
	result(3, 2) = 0.0f;
	result(3, 3) = 1.0f;

	return result;
}

inline
Mat44f make_translation( Vec3f aTranslation ) noexcept
{
	//TODO: your implementation goes here
	Mat44f result = kIdentity44f;

	result(0, 3) = aTranslation.x;
	result(1, 3) = aTranslation.y;
	result(2, 3) = aTranslation.z;

	return result;
}

inline
Mat44f make_scaling( float aSX, float aSY, float aSZ ) noexcept
{
	//TODO: your implementation goes here
	Mat44f result = kIdentity44f;

	result(0, 0) = aSX;
	result(1, 1) = aSY;
	result(2, 2) = aSZ;

	return result;
}

inline
Mat44f make_perspective_projection( float aFovInRadians, float aAspect, float aNear, float aFar ) noexcept
{
	//TODO: your implementation goes here
	Mat44f result = kIdentity44f;

	result(0, 0) = 1.0f / (aAspect * std::tan(aFovInRadians));
	result(0, 1) = 0.0f;
	result(0, 2) = 0.0f;
	result(0, 3) = 0.0f;

	result(1, 0) = 0.0f;
	result(1, 1) = 1.0f / std::tan(aFovInRadians);
	result(1, 2) = 0.0f;
	result(1, 3) = 0.0f;

	result(2, 0) = 0.0f;
	result(2, 1) = 0.0f;
	result(2, 2) = -(aFar + aNear) / (aFar - aNear);
	result(2, 3) = -2.0f * aFar * aNear / (aFar - aNear);

	result(3, 0) = 0.0f;
	result(3, 1) = 0.0f;
	result(3, 2) = -1.0f;
	result(3, 3) = 0.0f;

	return result;
}

inline Mat44f lookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up)
{
	auto f(normalize(center - eye));
	auto s(normalize(cross(f, up)));
	auto u(cross(s, f));

	Mat44f result = kIdentity44f;
	result(0, 0) = s.x;
	result(0, 1) = s.y;
	result(0, 2) = s.z;
	result(1, 0) = u.x;
	result(1, 1) = u.y;
	result(1, 2) = u.z;
	result(2, 0) = -f.x;
	result(2, 1) = -f.y;
	result(2, 2) = -f.z;
	result(0, 3) = -dot(s, eye);
	result(1, 3) = -dot(u, eye);
	result(2, 3) = dot(f, eye);
	return result;
}

#endif // MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
