#pragma once

#include "Body.h"

#pragma region SDL Includes
#include "SDL_keycode.h"
#pragma endregion

#pragma region Engine Includes
#include "IUpdatable.h"
#pragma endregion

/*
 * Class defining the behaviour of a
 * paddle in the PONG 2D game.
 */
class Paddle : public Body, public IUpdatable
{
private:
	int upperLimit = -9999;	//	Paddles have limited movement, this limits from above
	int lowerLimit = 9999;	//	Paddles have limited movement, this limits from below
	SDL_Keycode upKey = SDLK_UNKNOWN;
	SDL_Keycode downKey = SDLK_UNKNOWN;
public:
	using Body::Body;	//	This inherits base class' constructors
	//	Used to update limits
	__inline void SetLimits(int newUpperLimit, int newLowerLimit) { upperLimit = newUpperLimit; lowerLimit = newLowerLimit; }
	__inline void SetControl(const SDL_Keycode & up, const SDL_Keycode & down) { upKey = up; downKey = down; }
	//	Perform frame operations
	void Update() override;
private:
	//	Overriding this function to receive a message after each move
	virtual void PostMoveOperations() override;
};

