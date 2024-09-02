#include "SplashScreen.h"

#pragma region C++ Includes
#include <sstream>
#pragma endregion

#pragma region SDL Inlcudes
#include "SDL_image.h"
#pragma endregion

#pragma region Engine Includes
#include "Colors.h"
#include "Input.h"
#include "PathUtils.h"
#pragma endregion

#pragma region Constant Parameters
#define SPLASH_VERTICAL_FILL 1.0f
#pragma endregion

SplashScreen::SplashScreen(const SDL_Rect & viewport, const char * imageFileName, const Uint32 & duration) :
	startTime(SDL_GetTicks64()),
	viewport(viewport),
	duration(duration),
	color(SDLC_WHITE)
{
	SetImage(imageFileName);
}

SplashScreen::~SplashScreen()
{
	FlushImage();
}

bool SplashScreen::IsActive() const
{
	return !skip && SDL_GetTicks64() <= startTime + duration;
}

void SplashScreen::SetImage(const char * newImageFileName)
{
	imagePath = PathUtils::Combine(
		{
			SDL_GetBasePath(),
			"res",
			"img",
			"splash",
			PathUtils::AddImageExtension(newImageFileName)
		}
	);
}

void SplashScreen::Update()
{
	if(skip)
		return;

	if(
		Input::Get().GetKey(SDLK_SPACE) ||
		Input::Get().GetKey(SDLK_ESCAPE)
		)
		skip = true;
}

const SDL_Color & SplashScreen::GetColor() const
{
	return color;
}

const SDL_Rect SplashScreen::GetRect() const
{
	SDL_Rect targetRect;
	targetRect.h = (int)(viewport.h * SPLASH_VERTICAL_FILL);
	targetRect.w = (int)(targetRect.h * (textureSize.x / (float)textureSize.y));
	targetRect.x = (viewport.w / 2) - (targetRect.w / 2);
	targetRect.y = (viewport.h / 2) - (targetRect.h / 2);

	return targetRect;
}

void SplashScreen::PreRender(SDL_Renderer * r)
{
	if(
		IsActive() &&
		!imageTexture
	)
		LoadImage(r);
}

void SplashScreen::Render(SDL_Renderer * r) const
{
	SDL_Rect targetRect = GetRect();
	SDL_RenderCopy(r, imageTexture, nullptr, &targetRect);
}

void SplashScreen::LoadImage(SDL_Renderer * r)
{
	FlushImage();

#ifdef LOAD_SPLASH_VIA_SURFACE
		//	Load image file into a surface with pixel-level access
	SDL_Surface * splashImageSurface = IMG_Load(imagePath.c_str());
	if(splashImageSurface)
	{
		//	Create a texture and store texture size information
		splashImageTexture = SDL_CreateTextureFromSurface(r, splashImageSurface);
		// //	in this case it's faster to just read teh surface's type instead of querying the texture
		textureSize.x = splashImageSurface->w;
		textureSize.y = splashImageSurface->h;
		//	Free the surface
		SDL_FreeSurface(splashImageSurface);
	}
#else
		//	Load image file directly into a texture
	imageTexture = IMG_LoadTexture(r, imagePath.c_str());
	//	Query the texture for the relevant information (size, we omit format and access, which we don't need now)
	if(imageTexture)
		SDL_QueryTexture(imageTexture, nullptr, nullptr, &textureSize.x, &textureSize.y);
#endif
}

void SplashScreen::FlushImage()
{
	if(!imageTexture)
		return;

	SDL_DestroyTexture(imageTexture);
	imageTexture = nullptr;
}
