#include "Body.h"

#pragma region Engine Inlcudes
#include "Colors.h"
#pragma endregion


Body::Body(int width, int height, int bodySpeed) :
	t{},
	size{width, height},
	speed(bodySpeed)
{
	Init();
}

Body::Body(Vector2 bodySize, int bodySpeed) :
	t{},
	size(bodySize),
	speed(bodySpeed)
{
	Init();
}

void Body::Move(Vector2 offset)
{
	//	Change the position of the transform
	t.position += offset;

	//	Run post-move hook
	PostMoveOperations();
}

const SDL_Rect Body::GetRect() const
{
	/*
	 * Calculat ethe final rect in SDL-space based on
	 * size and pivot.
	 * SDL space has (0, 0) coordinates in the top-left
	 * corner of the render target. When drawing a rect,
	 * (x, y) refer to the rect's top-left corner's distance
	 * from the top-left corner of the render target,
	 * which isn't always the most handy situation.
	 * This function calculates the actual extents of the
	 * rect, taking into account size and scale, and
	 * offsets it by the pivot value, proportionally.
	 */
	SDL_Rect r;

	//	Calculate actual extents
	r.w = (int)(size.x * t.scale);
	r.h = (int)(size.y * t.scale);
	//	Offset by pivot, proportionally
	r.x = (int)(t.position.x - r.w * t.pivot.x);
	r.y = (int)(t.position.y - r.h * t.pivot.y);

	return r;
}

void Body::Render(SDL_Renderer * r) const
{
	/*
	 * Here we take care of the rendering to the
	 * back buffer. All IRenderables will write
	 * their own pixels on the current renderer
	 * but it will be the main loop to decide
	 * when to clear and when to swap the back
	 * and the front buffers.
	 * We're setting the color for this object
	 * but remember that alpha will be ignored
	 * if the current blend mode isn't reading
	 * from it, that's why, if alpha is not
	 * full opaque, we set the blend mode to
	 * SDL_BLENDMODE_BLEND which computes the
	 * final color as:
	 * (srcRGB * srcA) + (dstRGB * (1-srcA))
	 */
	SDL_SetRenderDrawColor(
		r,
		GetColor().r,
		GetColor().g,
		GetColor().b,
		GetColor().a
	);
	if(GetColor().a < 255)
		SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
	else
		SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
	SDL_Rect currentRect = GetRect();
	SDL_RenderFillRect(r, &currentRect);
}

void Body::Init()
{
	//	Set default pivot
	t.pivot.x = 0.5f;
	t.pivot.y = 0.5f;

	//	Set default color
	color = SDLC_WHITE;
}
