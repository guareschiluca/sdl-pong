#include "Paddle.h"

#pragma region Engine includes
#include "Input.h"
#pragma endregion

void Paddle::Update()
{
	int currentSpeed = 0;
	if(Input::Get().GetKey(upKey))
		currentSpeed = -speed;
	else if(Input::Get().GetKey(downKey))
		currentSpeed = speed;
	//Move up/down (or still if no direction imparted)
	Move(Vector2{0, currentSpeed});
}

void Paddle::PostMoveOperations()
{
	//	Calculate offsets relative to pivot
	int offsetUp = (int)(size.y * t.pivot.y);
	int offsetDown = (int)(size.y * (1.0f - t.pivot.y));

	//	Limit movement based on limits and pivot
	if(t.position.y < upperLimit + offsetUp)
		t.position.y = upperLimit + offsetUp;
	else if(t.position.y > lowerLimit - offsetDown)
		t.position.y = lowerLimit - offsetDown;
}
