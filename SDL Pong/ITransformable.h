#pragma once

#include "Transform.h"

#pragma region Axis utility enum
enum class Axis : int
{
	X = 1 << 0,
	Y = 1 << 1
};

/*
 * Operators here are made for testing purposes
 * and do not return an Axis instance, instead
 * they return the bitwise result for testing as
 * bool or against another int.
 */
inline int operator &(const Axis & lhs, const Axis & rhs)
{
	return static_cast<int>(lhs) & static_cast<int>(rhs);
}

inline int operator |(const Axis & lhs, const Axis & rhs)
{
	return static_cast<int>(lhs) | static_cast<int>(rhs);
}

inline int operator ^(const Axis & lhs, const Axis & rhs)
{
	return static_cast<int>(lhs) ^ static_cast<int>(rhs);
}
#pragma endregion

/*
 * Common interface for all objects with a
 * presence in the scene
 */
class ITransformable
{
	/*
	 * The two functions do the same but GetTransform() allows modification
	 * while ReadTransform() is for read-only operations and can be called
	 * from const functions and on const objects.
	 */

public:
	virtual Transform * GetTransform() = 0;
	virtual const Transform * ReadTransform() const = 0;
};

