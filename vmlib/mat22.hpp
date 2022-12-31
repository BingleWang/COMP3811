#ifndef MAT22_HPP_1F974C02_D0D1_4FBD_B5EE_A69C88112088
#define MAT22_HPP_1F974C02_D0D1_4FBD_B5EE_A69C88112088

#include <cmath>

#include "vec2.hpp"
#include "utils.hpp"

/** Mat22f : 2x2 matrix with floats
 *
 * See comments for Vec2f for some discussion.
 *
 * The matrix is stored in row-major order.
 *
 * Example:
 *   Mat22f identity{ 
 *     1.f, 0.f,
 *     0.f, 1.f
 *   };
 */
struct Mat22f
{
	float _00, _01;
	float _10, _11;
};

// Common operators for Mat22f.
// Note that you will need to implement these yourself.

constexpr
Mat22f operator*( Mat22f const& aLeft, Mat22f const& aRight ) noexcept
{
	//TODO: your implementation goes here
	//TODO: your implementation goes here
	//TODO: your implementation goes here
	Vec2f leftRow0 = { aLeft._00, aLeft._01 };
	Vec2f leftRow1 = { aLeft._10, aLeft._11 };
	Vec2f rightColumn0 = { aRight._00, aRight._10 };
	Vec2f rightColumn1 = { aRight._01, aRight._11 };

	auto m00 = dot(leftRow0, rightColumn0);
	auto m01 = dot(leftRow0, rightColumn1);
	auto m10 = dot(leftRow1, rightColumn0);
	auto m11 = dot(leftRow1, rightColumn1);

	//TODO: remove the following when you start your implementation
	
	return Mat22f{ m00, m01, m10, m11 };
}

constexpr
Vec2f operator*( Mat22f const& aLeft, Vec2f const& aRight ) noexcept
{
	//TODO: your implementation goes here
	//TODO: your implementation goes here
	//TODO: your implementation goes here
	Vec2f leftRow0 = { aLeft._00, aLeft._01 };
	Vec2f leftRow1 = { aLeft._10, aLeft._11 };
	auto result = aRight;
	result.x = dot(leftRow0, aRight);
	result.y = dot(leftRow1, aRight);

	//TODO: remove the following when you start your implementation

	return result;
}

// Functions:

inline
Mat22f make_rotation_2d( float aAngle ) noexcept
{
	//TODO: your implementation goes here
	//TODO: your implementation goes here
	//TODO: your implementation goes here


	//TODO: remove the following when you start your implementation

	return Mat22f{ std::cos(toRadians(aAngle)), -std::sin(toRadians(aAngle)), 
				   std::sin(toRadians(aAngle)),  std::cos(toRadians(aAngle)) };
}

#endif // MAT22_HPP_1F974C02_D0D1_4FBD_B5EE_A69C88112088
