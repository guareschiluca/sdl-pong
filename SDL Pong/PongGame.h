#pragma once

#pragma region C++ Includes
#include <vector>
#pragma endregion

#pragma region SDL Includes
#include "SDL.h"
#pragma endregion

#pragma region Engine Includes
#include "IRenderable.h"
#include "IUpdatable.h"
#include "Label.h"
#pragma endregion

#pragma region Game Includes
#include "SplashScreen.h"
#include "Body.h"
#include "Paddle.h"
#include "Ball.h"
#pragma endregion

class PongGame : public IRenderable, public IUpdatable
{
	// Fields
public:
protected:
private:
	SDL_Rect viewport;
	Body topBorder;
	Body bottomBorder;
	Body centerLine;
	Body goalP1;
	Body goalP2;
	Paddle padP1;
	Paddle padP2;
	Ball ball;
	int scoreP1 = 0;
	int scoreP2 = 0;
	Label scoreLabelP1;
	Label scoreLabelP2;

	SplashScreen * splashScreen;

	const vector<IRenderable *> renderQueue;
	const SDL_Color color;	//	Unused

#pragma region Input Mapping
	/*	KEY MAPPING	*/
	const SDL_Keycode upKeyP1 = SDLK_w;			//	Player 1 UP
	const SDL_Keycode downKeyP1 = SDLK_s;		//	Player 1 DOWN
	const SDL_Keycode upKeyP2 = SDLK_i;			//	Player 2 UP
	const SDL_Keycode downKeyP2 = SDLK_k;		//	Player 2 DOWN
	const SDL_Keycode kickOffKey = SDLK_SPACE;	//	Shared Kick-Off
	/*	===========	*/
#pragma endregion
	// Constructors
public:
	PongGame(const int & viewportWidth, const int & viewportHeight);
protected:
private:
	// Methods
public:
	//	IRenderable implementation
	const SDL_Color & GetColor() const override;
	const SDL_Rect GetRect() const override;
	void PreRender(SDL_Renderer * r) override;
	void Render(SDL_Renderer * r) const override;

	//	IUpdatable impementation
	void Update() override;
protected:
private:
	void PlaceBallToCenter();
};

