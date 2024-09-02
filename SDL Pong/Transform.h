#pragma once

#include "Types.h"

/*
 * 
 */
struct Transform
{
	//	The position of an object in a 2D discrete space
	Vector2 position;
	//	A scale value for the size of an object
	float scale;
	//	The reference point for the transforamtions, expressed in noramlized space relative to the size of the object
	Vector2F pivot;

	//	Constructor
	Transform();
};

