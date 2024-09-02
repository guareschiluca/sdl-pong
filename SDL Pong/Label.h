#pragma once

#include "ITransformable.h"
#include"ITextRenderable.h"

#include "SDL.h"

/*
 * Class that allows to display text in
 * a 2D space.
 */
class Label : public ITransformable, public ITextRenderable
{
private:
	Transform t;	//	Gives a label a place in 2D space
	string text;	//	The text displayed by the label
	SDL_Texture * fontTexture;	//	The cached texture of the rendered text
	SDL_Color color;	//	The color for the rendered text
	string fontPath = "";	//	The path to the font (relative to the base directory /res/fonts/ directory, with no extension, that will be added based on platform)
	Uint8 fontSize = 24;	//	The point size of the rendered font
	Vector2 size{0, 0};	//	The 2D size of the rendered font texture
	bool isDirty = false;	//	Set to true each time a change is made, when true, texture will be rendered anew

public:
	//	Constructor and destructor
	Label(string initialText, Uint8 initialFontSize);
	~Label();

	//	ITransformable implementation
	Transform * GetTransform() override { return &t; }
	const Transform * ReadTransform() const override { return &t; }

	//	IRenderable implementation + setters
	const SDL_Rect GetRect() const override;
	__inline const SDL_Color & GetColor() const override { return color; }
	__inline void SetColor(const SDL_Color & newColor) { SetColor(newColor.r, newColor.g, newColor.b, newColor.a); }
	void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
	void PreRender(SDL_Renderer * r) override;
	void Render(SDL_Renderer * r) const override;

	//	ITextRenderable implementation + setters
	const string & GetText() const override { return text; }
	void SetText(const string & newText);
	virtual const string & GetFontPath() const override { return fontPath; }
	void SetFontPath(const string & newFontPath);
	virtual const Uint8 & GetFontSize() const override { return fontSize; }
	void SetFontSize(Uint8 newFontSize);

private:
	__inline void SetDirty() { isDirty = true; }
	__inline void CleanDirty() { isDirty = false; }
	__inline bool IsDirty() const { return isDirty; }
	void ClearTexture();
	//	Creates the font texture based on the current object's state
	void RenderCurrentText(SDL_Renderer * r);
};

