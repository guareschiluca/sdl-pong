#include "Ball.h"

#pragma region C++ Includes
#include <random>
#include <iostream>
#include <sstream>
#include <cmath>
#pragma endregion

#pragma region SDL Includes
#include "SDL.h"
#include "SDL_mixer.h"
#pragma endregion

#pragma region Engine Includes
#include "PathUtils.h"
#pragma endregion

#define Sign(number) (number >= 0 ? 1 : -1)

using namespace std;

Ball::~Ball()
{
	//	Free mixer chunks' memory
	FreeChunk(obstacleSFX);
	FreeChunk(paddleSFX);
	FreeChunk(goalSFX);
}

void Ball::SetDirection(BallDirection newDirection)
{
	direction = newDirection;
}

void Ball::RandomizeDirection()
{
	/*
	 * Random in C and C++ isn't trivial as we're used to
	 * with C#.
	 * C had its own way to generate random numbers, based
	 * on a pseudo-random function and a seed, a number on
	 * which calculate a sequence of random numbers. This
	 * seed could be taken from different sources, the most
	 * common was the current timestamp.
	 * C++ introduced a more precise random number generation
	 * method, based on a random engine and a distribution.
	 * The random engine follows the same seed logic as C
	 * and a good way to get a random number is to take the
	 * id of a random device.
	 */
	random_device rd;
	default_random_engine engine(rd());
	uniform_int_distribution<int> random(1, 4);

	SetDirection((BallDirection)random(engine));
}

void Ball::Place(int x, int y)
{
	t.position.x = x;
	t.position.y = y;
	SetDirection(BD_Still);
}

void Ball::KickOff()
{
	if(direction == BD_Still)
		RandomizeDirection();
}

void Ball::FlipDirectionV()
{
	switch(direction)
	{
		case BD_NE:
			SetDirection(BD_SE);
			break;
		case BD_NW:
			SetDirection(BD_SW);
			break;
		case BD_SE:
			SetDirection(BD_NE);
			break;
		case BD_SW:
			SetDirection(BD_NW);
			break;
		default:
			//	Do nothing
			break;
	}
}

void Ball::FlipDirectionH()
{
	switch(direction)
	{
		case BD_NE:
			SetDirection(BD_NW);
			break;
		case BD_NW:
			SetDirection(BD_NE);
			break;
		case BD_SE:
			SetDirection(BD_SW);
			break;
		case BD_SW:
			SetDirection(BD_SE);
			break;
		default:
			//	Do nothing
			break;
	}
}

const Body * Ball::ConsumePoint()
{
	const Body * pointCache = point;
	point = nullptr;
	return pointCache;
}

void Ball::Update()
{
	if(point)
		return;

	Vector2 moveDir{0, 0};

	if(
		direction == BD_NE ||
		direction == BD_SE
		)
		moveDir.x++;
	else if(
		direction == BD_NW ||
		direction == BD_SW
		)
		moveDir.x--;

	if(
		direction == BD_NE ||
		direction == BD_NW
		)
		moveDir.y--;
	else if(
		direction == BD_SE ||
		direction == BD_SW
		)
		moveDir.y++;

	moveDir *= speed;

	Move(moveDir);
}

void Ball::PostMoveOperations()
{
	//	Store current presence in scene
	SDL_Rect currentRect = GetRect();

	//	Check intersections with goals to determine points
	for(const Body * const & goal : goals)
	{
		SDL_Rect goalRect = goal->GetRect();
		if(SDL_HasIntersection(&currentRect, &goalRect))
		{
			SetDirection(BD_Still);
			point = goal;
			PlaySFX(goalSFX, 2);
			return;
		}
	}

	/*
	 * To detect collisions we're actually detecting overlaps.
	 * This is a really approximate collision detection algorithm,
	 * which is not physically accurate since physics says that
	 * two bodies cannot occupy the same space at the same time.
	 * In order to avoid flickering and stale situations, when
	 * an collisio (overlap in this case) happens, we need to
	 * make little adjustments to the ball's position to
	 * prevent the possibility that on the next update the
	 * overlap persists.
	 * Here we're doing it in a very simplified way, taking
	 * advantage from the simplicity of the game world:
	 * - we know what collisions can happen
	 * - collisions are always the same througout the game
	 * - two objects will always collide the same way
	 *		- obstacles vertically
	 *		- paddles horizontally
	 * - the speed of the ball is fixed and low
	 * Based on those (many) assumptions, resolving overlaps
	 * becomes really easy. We consider only the axis we know
	 * is causing an overlap (x for paddles, y for obstacles),
	 * we calculate the minimum distance on that axis for the
	 * two bodies to not overlap and move the ball at that
	 * exact distance, depending on the side of the ball
	 * relative to the other body.
	 * 
	 * A big problem here, that will come out for sure if
	 * we increase the ball's speed, is that this is not a
	 * physical simulation and ignores the velocity and where
	 * the ball comes from, making a discrete collision detection
	 * that can lead to ofershooting the other body.
	 * Implementing a continuous collision detection wouldn't
	 * be that hard but it would require a little refactoring
	 * to pass informatin about the position of the ball
	 * before it moved, that, in conjunction with the current
	 * position of the ball itself, gives an information of
	 * movement and so it would be possible to intersect the
	 * movement with the other body or simply to check if the
	 * sign of the relative position changes before and after
	 * the movement.
	 */

	//	Check intersections with obstacles to bounce away
	for(const Body * const & obstacle : obstacles)
	{
		SDL_Rect obstacleRect = obstacle->GetRect();
		if(SDL_HasIntersection(&currentRect, &obstacleRect))
		{
			//	Bounce up<->down
			FlipDirectionV();

			//	Resolve compenetrations
			ResolveOverlap(currentRect, obstacleRect, Axis::Y);

			//	Play obstacle bounce sound
			PlaySFX(obstacleSFX, 0);
			break;
		}
	}

	//	Check intersection with paddles to bounce away
	for(const Body * const & paddle : paddles)
	{
		SDL_Rect obstacleRect = paddle->GetRect();
		if(SDL_HasIntersection(&currentRect, &obstacleRect))
		{
			//	Bounce left<->right
			FlipDirectionH();

			//	Resolve compenetrations
			ResolveOverlap(currentRect, obstacleRect, Axis::X);

			//	Play paddle bounce sound
			PlaySFX(paddleSFX, 1);
			break;
		}
	}
}

void Ball::LoadMixerChunk(const char * & chunkSfxPath, Mix_Chunk * & destination)
{
	//	Free memory for the possible currently loaded sfx
	FreeChunk(destination);

	//	Build the full path
	string fullPath = PathUtils::Combine(
		{
			SDL_GetBasePath(),
			"res",
			"sound",
			"sfx",
			PathUtils::AddChunkExtension(chunkSfxPath)
		}
	);

	//	Load chunk from wav file
	destination = Mix_LoadWAV(fullPath.c_str());

	if(!destination)
		cout << "Couldn't load sound effect at " << fullPath << " [ERROR]: " << Mix_GetError() << endl;
}

void Ball::PlaySFX(Mix_Chunk * & sfx, int channel)
{
	//	Check if there is any chunk to play
	if(!sfx)
		return;

	//	Play the chunk on the given channel once
	Mix_PlayChannel(channel, sfx, 0);
}

void Ball::FreeChunk(Mix_Chunk * & sfx)
{
	if(!sfx)
		return;

	Mix_FreeChunk(sfx);
	sfx = nullptr;
}

void Ball::ResolveOverlap(const SDL_Rect & currentRect, const SDL_Rect & obstacleRect, Axis axis)
{
	//	Resolve overlap on every requested axis
	if(axis & Axis::X)
	{
		//	Get axis shift
		const int shift = GetOverlapShift(
			currentRect.x,
			currentRect.w / 2,
			obstacleRect.x,
			obstacleRect.w / 2
		);

		//	Apply shift to X axis
		t.position += Vector2(shift, 0);
	}
	if(axis & Axis::Y)
	{
		//	Get axis shift
		const int shift = GetOverlapShift(
			currentRect.y,
			currentRect.h / 2,
			obstacleRect.y,
			obstacleRect.h / 2
		);

		//	Apply shift to Y axis
		t.position += Vector2(0, shift);
	}
}

int Ball::GetOverlapShift(int currentPos, int currentExtent, int obstaclePos, int obstacleExtent) const
{
	//	Calculate centers
	const int currentCenter = currentPos + currentExtent;
	const int obstacleCenter = obstaclePos + obstacleExtent;

	//	Cauclate current and required offset
	const int currentOffset = currentCenter - obstacleCenter;
	const int minOffset = currentExtent + obstacleExtent;

	/*
	 * In the calculation of the final shift, we could add
	 * a small margin to resolve both the overlap and the
	 * contact itself.
	 * For the purposes of this simple project, this has not
	 * been done because the next iteration will move the ball
	 * before checking any possible contact/overlap, and when
	 * the overlap is resolved also the velocity is inverted,
	 * so the natural course of the events shouldn't require
	 * any safety margin.
	 */

	//	Return the relative offset on this axis to resolve the overlap
	return Sign(currentOffset) * abs(minOffset -currentOffset);
}
