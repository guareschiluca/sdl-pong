#include "PongGame.h"

#pragma region Engine Includes
#include "Types.h"
#include "Colors.h"
#include "Input.h"
#pragma endregion

#pragma region Constant Parameters
//	Splash screen
#ifdef _DEBUG
#define SPLASH_DURATION 1000
#else
#define SPLASH_DURATION 2500
#endif
//	Field layout metrics
#define BORDERS_SIZE 10
#define CENTERLINE_SIZE 2
#define GOALS_SIZE 5
//	Gameplay layout metrics
#define BALL_SIZE 10
#define BALL_SPEED 8
#define PADDLES_SIZE 100
#define PADDLES_SPEED 5
#define PADDLES_GOAL_DISTANCE 30
#define PADDLES_BORDER_OFFSET (GOALS_SIZE + PADDLES_GOAL_DISTANCE + BALL_SIZE / 2)
//	HUD metrics
#define SCORE_FONT_SIZE 72
#define SCORE_TOP BORDERS_SIZE * 3
//	Media
#define MEDIA_IMG_SPLASH_SCREEN "SDLPONG_Cover_16_9"
#pragma endregion


PongGame::PongGame(const int & viewportWidth, const int & viewportHeight) :
	viewport{0, 0, viewportWidth, viewportHeight},
	topBorder{viewportWidth, BORDERS_SIZE},
	bottomBorder{viewportWidth, BORDERS_SIZE},
	centerLine{CENTERLINE_SIZE, viewportHeight},
	goalP1{GOALS_SIZE, viewportHeight},
	goalP2{GOALS_SIZE, viewportHeight},
	padP1{BALL_SIZE, PADDLES_SIZE, PADDLES_SPEED},
	padP2{BALL_SIZE, PADDLES_SIZE, PADDLES_SPEED},
	ball{BALL_SIZE, BALL_SIZE, BALL_SPEED},
	scoreLabelP1{to_string(scoreP1), SCORE_FONT_SIZE},
	scoreLabelP2{to_string(scoreP2), SCORE_FONT_SIZE},
	renderQueue
	{
#ifdef _DEBUG
		/*
		 * Renderables added here are rendered
		 * behind of everyting and only in
		 * debug builds.
		 */
		//	Display goals for debug
		&goalP1,
		&goalP2,
#endif
		//	Centerline behind all
		&centerLine,
		//	Score behind all but centerline
		&scoreLabelP1,
		&scoreLabelP2,
		//	Ball behind pads
		&ball,
		//	Pads above ball
		&padP1,
		&padP2,
		//	Borders above all
		&topBorder,
		&bottomBorder
	},
	color(SDLC_CLEAR)	//	Unused
{
	//	Initialize field elements
	topBorder.GetTransform()->pivot = Vector2F(0.5f, 0.0f);
	topBorder.GetTransform()->position = Vector2(viewportWidth / 2, 0);
	bottomBorder.GetTransform()->pivot = Vector2F(0.5f, 1.0f);
	bottomBorder.GetTransform()->position = Vector2(viewportWidth / 2, viewportHeight);
	centerLine.GetTransform()->position = Vector2(viewportWidth / 2, viewportHeight / 2);
	centerLine.SetColor(SDLC_GRAY);

	//	Initialize gameplay elements
	goalP1.GetTransform()->pivot = Vector2F(0.0f, 0.5f);
	goalP1.GetTransform()->position = Vector2(0, viewportHeight / 2);
#ifdef _DEBUG
	goalP1.SetColor(SDLC_GREEN);
#endif
	goalP2.GetTransform()->pivot = Vector2F(1.0f, 0.5f);
	goalP2.GetTransform()->position = Vector2(viewportWidth, viewportHeight / 2);
#ifdef _DEBUG
	goalP2.SetColor(SDLC_GREEN);
#endif
	padP1.GetTransform()->position = Vector2(PADDLES_BORDER_OFFSET, viewportHeight / 2);
	padP1.SetLimits(5 + BORDERS_SIZE, viewportHeight - 5 - BORDERS_SIZE);
	padP1.SetControl(upKeyP1, downKeyP1);
	padP2.GetTransform()->position = Vector2(viewportWidth - PADDLES_BORDER_OFFSET, viewportHeight / 2);
	padP2.SetLimits(5 + BORDERS_SIZE, viewportHeight - 5 - BORDERS_SIZE);
	padP2.SetControl(upKeyP2, downKeyP2);
	ball.SetColor(200, 50, 50);
	PlaceBallToCenter();

	//	Initialize collision detection
	ball.AddGoal(&goalP1);
	ball.AddGoal(&goalP2);

	ball.AddPaddle(&padP1);
	ball.AddPaddle(&padP2);

	ball.AddObstacle(&topBorder);
	ball.AddObstacle(&bottomBorder);

	//	Initialize HUD
	scoreLabelP1.GetTransform()->pivot = Vector2F(1.0f, 0.0f);
	scoreLabelP1.GetTransform()->position  = Vector2(viewportWidth / 2 - BORDERS_SIZE, SCORE_TOP);
	scoreLabelP1.SetColor(SDLC_GRAY);
	scoreLabelP2.GetTransform()->pivot = Vector2F(0.0f, 0.0f);
	scoreLabelP2.GetTransform()->position  = Vector2(viewportWidth / 2 + BORDERS_SIZE, SCORE_TOP);
	scoreLabelP2.SetColor(SDLC_GRAY);

	//	Initialize ball sounds
	ball.SetObstacleSFX("HitObstacle");
	ball.SetPaddleSFX("HitPaddle");
	ball.SetGoalSFX("TriggerGoal");

	//	Initialize splahs screen
	splashScreen = new SplashScreen(viewport, MEDIA_IMG_SPLASH_SCREEN, SPLASH_DURATION);
}

const SDL_Color & PongGame::GetColor() const
{
	// Unused but needed to implement interface
	return color;
}

const SDL_Rect PongGame::GetRect() const
{
	// Unused but needed to implement interface
	return viewport;
}

void PongGame::PreRender(SDL_Renderer * r)
{
	//	Pre-render splash screen when active
	if(
		splashScreen &&
		splashScreen->IsActive()
		)
		splashScreen->PreRender(r);
	else
		//	Pre-render game after splash screen
		for(IRenderable * const & renderable : renderQueue)
			renderable->PreRender(r);
}

void PongGame::Render(SDL_Renderer * r) const
{
	//	Render splash screen when active
	if(
		splashScreen &&
		splashScreen->IsActive()
		)
		splashScreen->Render(r);
	else
		//	Render game after splash screen
		for(IRenderable * const & renderable : renderQueue)
			renderable->Render(r);
}

void PongGame::Update()
{
	//	If splash screen is active, update it
	if(
		splashScreen &&
		splashScreen->IsActive()
	)
	{
		splashScreen->Update();
		return;
	}
	//	If splash screen is inactive but still exists, delete it
	if(splashScreen)
	{
		delete splashScreen;
		splashScreen = nullptr;
	}

	/*
	 * This approach is approximate as this
	 * gets triggered every frame until the
	 * kick off key is held.
	 * It's working because the KickOff()
	 * function checks the state of the ball
	 * but in a real world scenario it would
	 * be necessary to implement GetKeyUp()
	 * and GetKeyDown().
	 */
	if(Input::Get().GetKey(kickOffKey))
		ball.KickOff();

	//	Feed update to single components
	padP1.Update();
	padP2.Update();
	ball.Update();

	//	Check game state
	if(ball.HasPoint())
	{
		const Body * point = ball.ConsumePoint();

		if(point == &goalP2)
			scoreLabelP1.SetText(to_string(++scoreP1));
		else if(point == &goalP1)
			scoreLabelP2.SetText(to_string(++scoreP2));

		PlaceBallToCenter();
	}
}

void PongGame::PlaceBallToCenter()
{
	ball.Place(viewport.w / 2, viewport.h / 2);
}
