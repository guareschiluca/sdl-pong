#include "Label.h"

#pragma region C++ Includes
#ifdef _DEBUG
#include <iostream>
#endif
#pragma endregion

#pragma region SDL Includes
#include "SDL_ttf.h"
#pragma endregion

#pragma region Engine Includes
#include "PathUtils.h"
#pragma endregion


Label::Label(string initialText, Uint8 initialFontSize) :
	color{255, 255, 255, 255},
	fontSize(initialFontSize)
{
	SetFontPath("8bit16");
	SetText(initialText);	//	Triggers a render of the font texture
}

Label::~Label()
{
	//	Remember to free texture to avoid memory leaks
	ClearTexture();
}

const SDL_Rect Label::GetRect() const
{
	SDL_Rect r;

	r.w = (int)(size.x * t.scale);
	r.h = (int)(size.y * t.scale);
	r.x = (int)(t.position.x - r.w * t.pivot.x);
	r.y = (int)(t.position.y - r.h * t.pivot.y);

	return r;
}

void Label::SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;

	SetDirty();	//	Schedules a render of the font texture
}

void Label::PreRender(SDL_Renderer * r)
{
	/*
	 * This is the core of the optimization of this class.
	 * Pre-render is a stage of the frame lyfecycle after
	 * all updates and before the render. All functions
	 * that modify the state of this class (which are
	 * typically called during the update stage) set
	 * a dirty flag.
	 * To render and cache the texture upon changes, we
	 * need a reference to a renderer, which we do not have
	 * during the update stage. We would have it during the
	 * render stage, but, by design, render must not change
	 * anything (it's a const function) so the pre-render
	 * stage is the best place where we have a renderer and
	 * we can actually make changes.
	 * See RenderCurrentText() for more details.
	 */

	//	if any change has been made, render the texture
	if(IsDirty())
	{
		RenderCurrentText(r);
		CleanDirty();
	}
}

void Label::Render(SDL_Renderer * r) const
{
	/*
	 * This function limits its execution to making
	 * a copy of the pre-rendered font texture to
	 * the assigned render target.
	 * The actual render occurs only when any of the
	 * parameters affecting the render result changes.
	 * If a render was not done yet (shouldn't happen)
	 * the function does nothing but, in debug mode,
	 * draws a magenta marker indicating an error
	 * in text rendering.
	 */

	if(!fontTexture)
	{
#ifdef _DEBUG
		SDL_SetRenderDrawColor(r, 255, 0, 255, 255);
		SDL_Rect rect = GetRect();
		SDL_RenderDrawRect(r, &rect);
#endif
		return;
	}

	//	Copy the cached texture into the right portion of the render target
	SDL_Rect currentRect = GetRect();
	SDL_RenderCopy(r, fontTexture, nullptr, &currentRect);
}

void Label::SetText(const string & newText)
{
	text = newText;
	SetDirty();	//	Schedules a render of the font texture
}

void Label::SetFontPath(const string & newFontPath)
{
	fontPath = PathUtils::Combine(
		{
			SDL_GetBasePath(),
			"res",
			"fonts",
			PathUtils::AddFontExtension(newFontPath)
		}
	);
	SetDirty();	//	Schedules a render of the font texture
}

void Label::SetFontSize(Uint8 newFontSize)
{
	fontSize = newFontSize;
	SetDirty();	//	Schedules a render of the font texture
}

void Label::ClearTexture()
{
	if(!fontTexture)
		return;

	SDL_DestroyTexture(fontTexture);
	fontTexture = nullptr;
}

void Label::RenderCurrentText(SDL_Renderer * r)
{
	/*
	 * This function actually renders the current state of the
	 * Label class to a texture. Doing this operation each frame
	 * isn't necessary as most of time the text doesn't change.
	 * For this reason, we decided to call this function only
	 * when strictly needed (aka, when the state of the class
	 * changes).
	 * See PreRender() for further details.
	 */
	//	Build the absolute path of the font to load
	string fullPath = GetFontPath();

	//	Load the font object
	TTF_Font * font = TTF_OpenFont(fullPath.c_str(), (int)GetFontSize());

	//	Handle font loading errors
	if(!font)
	{
#ifdef _DEBUG
		size.x = GetFontSize();
		size.y = GetFontSize();
		cout << "--> Couldn't load font \"" << fullPath << "\" with size " << (int)GetFontSize() << endl;
		cout << "--> TTF Error: " << TTF_GetError() << endl;
#endif
		return;
	}

	//	Render the text to a surface and store the final size
	SDL_Surface * surf = TTF_RenderText_Blended(font, GetText().c_str(), GetColor());	//	Using the *_Blended version to allow transparent background
	size.x = surf->w;
	size.y = surf->h;

	//	Crete a texture from the surface (free previous cached texture, if any)
	ClearTexture();
	fontTexture = SDL_CreateTextureFromSurface(r, surf);

	//	Handle font rendering errors
	if(!font)
	{
#ifdef _DEBUG
		size.x = GetFontSize();
		size.y = GetFontSize();
		cout << "--> Font loaded but couldn't render it to texture." << endl;
		cout << "--> Font Rendering Error: " << SDL_GetError() << endl;
#endif
		return;
	}

	//	Free up surface memory
	SDL_FreeSurface(surf);
	//	Free up font memory
	TTF_CloseFont(font);
}
