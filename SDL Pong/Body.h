#pragma once

#include "SDL.h"

#include "ITransformable.h"
#include "IRenderable.h"

/*
 * Generic class for bodies with a rectangular shape
 * that can be placed and moved in the scene as well
 * as rendered.
 */
class Body : public ITransformable, public IRenderable
{
protected:
	Transform t;	//	Gives a body a place in 2D space
	Vector2 size;	//	Gives a body a presence in 2D space
	SDL_Color color;	//	Gives a body a color
	int speed;	//	Determines at what speed the body moves in the scene

public:
	//	Constructors
	Body(int width, int height, int bodySpeed = 0);
	Body(Vector2 bodySize, int bodySpeed = 0);
	
	//	ITransformable implementation + movement
	Transform * GetTransform() override { return &t; }
	const Transform * ReadTransform() const override { return &t; }
	void Move(Vector2 offset);
	
	//	IRenderable implementation + setters
	const SDL_Color & GetColor() const override { return color; }
	__inline void SetColor(SDL_Color newColor) { color = newColor; }
	__inline void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) { color.r = r; color.g = g; color.b = b; color.a = a; }
	const SDL_Rect GetRect() const override;
	void Render(SDL_Renderer * r) const override;
private:
	//	Initialization function with common operations to be called by all constructors
	void Init();
	//	Called after Move(), can be overridden in sub-classes to perform checks after movements
	virtual void PostMoveOperations() { }
};

