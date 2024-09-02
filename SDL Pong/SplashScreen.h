#pragma once

#pragma region C++ Includes
#include <string>
#pragma endregion

#pragma region SDL Includes
#include "SDL.h"
#pragma endregion

#pragma region Engine Includes
#include "IUpdatable.h"
#include "IRenderable.h"
#pragma endregion

using namespace std;

class SplashScreen : public IUpdatable, public IRenderable
{
	// Fields
public:
protected:
private:
	const SDL_Rect & viewport;	//	Reference is linked to the original viewport, changes will propagate here automatically
	const SDL_Color color;	//	Unused
	const Uint64 startTime;
	const Uint32 duration;
	string imagePath;
	SDL_Texture * imageTexture = nullptr;
	Vector2 textureSize{0, 0};
	bool skip = false;
	// Constructors
public:
	SplashScreen(const SDL_Rect & viewport, const char * imageFileName, const Uint32 & duration);
	~SplashScreen();
protected:
private:
	// Methods
public:
	bool IsActive() const;
	void SetImage(const char * newImageFileName);

	//	IUpdatable implementation
	void Update() override;

	//	IRenderable implementation
	const SDL_Color & GetColor() const override;
	const SDL_Rect GetRect() const override;
	void PreRender(SDL_Renderer * r) override;
	void Render(SDL_Renderer * r) const override;
protected:
private:
	void LoadImage(SDL_Renderer * r);
	void FlushImage();
};

