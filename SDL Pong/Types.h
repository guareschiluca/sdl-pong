#pragma once

/*
 * This file contains a set of common types.
 */

//	2D Vector type, used to identify positions, sizes and velocities in a 2D discrede space
typedef struct Vector2
{
	int x;
	int y;

	Vector2(int xVal, int yVal) : x(xVal), y(yVal) { }

	Vector2 operator *(int const & scale) const
	{
		return Vector2{x * scale, y * scale};
	}
	Vector2 & operator *=(int const & scale)
	{
		this->x *= scale;
		this->y *= scale;

		return *this;
	}

	Vector2 operator *(float const & scale) const
	{
		return Vector2{(int)(x * scale), (int)(y * scale)};
	}
	Vector2 & operator *=(float const & scale)
	{
		this->x = (int)(this->x * scale);
		this->y = (int)(this->y * scale);

		return *this;
	}

	Vector2 operator +(Vector2 const & other) const
	{
		return Vector2{x + other.x, y + other.y};
	}
	Vector2 & operator +=(Vector2 const & other)
	{
		this->x += other.x;
		this->y += other.y;

		return *this;
	}
} Vector2;

//	2D Vector type, used to identify positions, sizes and velocities in a 2D continuous space
typedef struct Vector2F
{
	float x;
	float y;

	Vector2F(float xVal, float yVal) : x(xVal), y(yVal) { }
} Vector2F;
