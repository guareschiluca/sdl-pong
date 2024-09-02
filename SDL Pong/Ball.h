#pragma once

#include "Body.h"

#pragma region C++ Includes
#include <vector>
#pragma endregion

#pragma region Engine Includes
#include "IUpdatable.h"
#pragma endregion

using namespace std;

typedef enum
{
	BD_Still	= 0,
	BD_NE		= 1,
	BD_NW		= 2,
	BD_SE		= 3,
	BD_SW		= 4
} BallDirection;

/*
 * Class defining the behaviour of the
 * ball in the PONG 2D game.
 */
class Ball : public Body, public IUpdatable
{
private:
	BallDirection direction = BD_Still;
	vector<const Body *> paddles;
	vector<const Body *> obstacles;
	vector<const Body *> goals;
	const Body * point = nullptr;
	struct Mix_Chunk * obstacleSFX;
	struct Mix_Chunk * paddleSFX;
	struct Mix_Chunk * goalSFX;

public:
	using Body::Body;	//	This inherits base class' constructors
	~Ball();
	//	Gives the ball a direction to follow
	void SetDirection(BallDirection newDirection);
	//	Gives the ball a random direction to follow
	void RandomizeDirection();
	//	Stops the ball in a given place
	void Place(int x, int y);
	//	Sets a random direction to the ball, only if the ball is still
	void KickOff();
	//	Flips the velocity of the ball on the vertical axis
	void FlipDirectionV();
	//	Flips the velocity of the ball on the horizontal axis
	void FlipDirectionH();
	//	Registers a paddle for collision check
	__inline void AddPaddle(class Body const * newPaddle) { paddles.push_back(newPaddle); }
	//	Registers an obstacle for collision check
	__inline void AddObstacle(class Body const * newObstacle) { obstacles.push_back(newObstacle); }
	//	Registers a goal for trigger check
	__inline void AddGoal(class Body const * newGoal) { goals.push_back(newGoal); }
	__inline bool HasPoint() const { return point != nullptr; }
	__inline const Body * PeekPoint() const { return point; }
	//	Check if the ball scored a point on any goal. If nullptr, no point was scored. Point is cleared on read, use HasPoint() PeekPoint() if you wanna read without resetting.
	const Body * ConsumePoint();
	__inline void SetObstacleSFX(const char * sfxPath) { LoadMixerChunk(sfxPath, obstacleSFX); }
	__inline void SetPaddleSFX(const char * sfxPath) { LoadMixerChunk(sfxPath, paddleSFX); }
	__inline void SetGoalSFX(const char * sfxPath) { LoadMixerChunk(sfxPath, goalSFX); }
	//	Perform frame operations
	void Update() override;
private:
	//	Overriding this function to receive a message after each move
	virtual void PostMoveOperations() override;
	void LoadMixerChunk(const char * & chunkSfxPath, struct Mix_Chunk * & destination);
	void PlaySFX(struct Mix_Chunk * & sfx, int channel = -1);
	void FreeChunk(struct Mix_Chunk * & sfx);
	void ResolveOverlap(const SDL_Rect & currentRect, const SDL_Rect & obstacleRect, Axis axis);
	int GetOverlapShift(int currentPos, int currentExtent, int obstaclePos, int obstacleExtent) const;
};

