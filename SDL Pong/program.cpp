#pragma region C++ Includes
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#pragma endregion

#pragma region SDL Includes
//	SDL Core
#include "SDL.h"

//	SDL Modules
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#pragma endregion

#pragma region Engine Includes
#include "Types.h"	//	Set of utility types
#include "IUpdatable.h"	//	Interface used in the update/logic loop
#include "IRenderable.h"	//	Interface used in the render loop
#include "Input.h"	//	Singleton that manages and exposes input events
#include "PathUtils.h"	//	Utilities for cross-platform paths handing
#pragma endregion

#pragma region Game Includes
//	Game elements
#include "PongGame.h"

#include "Paddle.h"
#include "Ball.h"
#include "Label.h"
#pragma endregion

#pragma region Emscripten Includes
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#pragma endregion

using namespace std;
using namespace std::chrono;

#pragma region Docs Links
/*
 * SDL Resources
 *		Lifecycle: https://wiki.libsdl.org/SDL2/CategoryInit
 *		Error Handling: https://wiki.libsdl.org/SDL2/CategoryError
 *		General API: https://wiki.libsdl.org/SDL2/APIByCategory
 *		TTF API: https://wiki.libsdl.org/SDL2_ttf/CategoryAPI
 *		Image API: https://wiki.libsdl.org/SDL2_image/CategoryAPI
 *		Mixer API: https://wiki.libsdl.org/SDL2_mixer/CategoryAPI
 */
#pragma endregion

#pragma region Constant Parameters
/*
 * Viewport should be Full-HD and full screen
 * in release builds, and windowed HD-Ready
 * in debug builds, to ease debug operations.
 */
#ifdef _DEBUG
#define VIEWPORT_W 1280
#define VIEWPORT_H 720
#define VIEWPORT_MODE SDL_WINDOW_RESIZABLE
#else
#ifdef __EMSCRIPTEN__
#define VIEWPORT_W 800
#define VIEWPORT_H 450
#define VIEWPORT_MODE SDL_WINDOW_RESIZABLE
#elif
#define VIEWPORT_W 1920
#define VIEWPORT_H 1080
#define VIEWPORT_MODE SDL_WINDOW_FULLSCREEN
#endif
#endif
#define SDL_INIT_MODE (SDL_INIT_VIDEO | SDL_INIT_AUDIO)
#ifndef __EMSCRIPTEN__
#define IMG_INIT_MODE IMG_INIT_PNG | IMG_INIT_JPG
#else
/*
 * When targetting webgl, we let emscripten use browser's
 * codecs to decode image formats so we do not need to
 * initialize support for them.
 * This is also useful since it doesn't require us to
 * build a custom version of SDL2_image with support
 * for the different formats and, instead, use emscripten's
 * port of SDL2_image.
 */
#define IMG_INIT_MODE 0
#endif
#ifndef __EMSCRIPTEN__
#define MIX_INIT_MODE MIX_INIT_MP3 | MIX_INIT_WAVPACK
#else
/*
 * When targetting webgl, we let emscripten use browser's
 * codecs to decode audio formats so we do not need to
 * initialize support for them.
 * This is also useful since it doesn't require us to
 * build a custom version of SDL2_mixer with support
 * for the different formats and, instead, use emscripten's
 * port of SDL2_mixer.
 */
#define MIX_INIT_MODE MIX_INIT_MP3
#endif

//	The fixed time step we aim to
#define TARGET_FPS 60
#define TARGET_FRAME_TIME 1000 / TARGET_FPS

#define RENDER_CLEAR_COLOR 10, 10, 10, 255

#ifdef _DEBUG
#define BGM_FADE_IN_TIME 1000
#else
#define BGM_FADE_IN_TIME 2500
#endif
#define BGM_FADE_OUT_TIME 500

#ifdef __EMSCRIPTEN__
#define HTML_CANVAS_SELECTOR "#canvas"
#endif
#pragma endregion

#pragma region Exchange data
typedef struct
{
	SDL_Window * window;
	SDL_Renderer * r;
	int viewportWidth;
	int viewportHeight;

} SystemData;
typedef struct
{
	bool closeRequested;
	vector<IUpdatable *> updateQueue;
	vector<IRenderable *> renderQueue;
} EngineData;
typedef struct
{
	PongGame * pongGame;
	Mix_Music * bgm;
} GameData;
typedef struct
{
	SystemData system;
	EngineData engine;
	GameData game;
} Context;
#pragma endregion

//	Forward declarations
int SystemSetup();
void StartMusic();
void MainLoop();
void StopMusic();
void SystemShutdown();

//	Prepare a global context for the main loop and the main function
Context ctx;

/*	ENTRY POINT	*/
int main(int argc, char * argv[])
{
#pragma region System Setup
	/*
	 * Here we're going to initialize and set up
	 * the main elements that will make our game
	 * work, such as SDL itself, font support, the
	 * game window and the renderer.
	 */
	const int setupResult = SystemSetup();
	if(setupResult != 0)
		return setupResult;

	StartMusic();
#pragma endregion

#pragma region Console splash screen
#ifdef _DEBUG
	/*
	 * Just a cool way to say that everything
	 * initialized successfully.
	 * Read the contents of a raw resource
	 * (a text file) and print it line by line.
	 */
	//	Confirm all initialization operations completed successfully
	cout << "System up and running!" << endl << endl;
	{
		//	Build the full path for the title raw resource
		ostringstream titleFullPath;
		titleFullPath << SDL_GetBasePath() << "res\\" << "raw\\title.aa";

		//	Open the title resource as input for read
		ifstream inputFile(titleFullPath.str());
		if(inputFile)
		{
			//	Read the file line by line and print that line
			string line;
			while(getline(inputFile, line))
				cout << line << endl;

			//	Close the file handle (no need to close it if the open failed, that's why this is inside the if block)
			inputFile.close();
		}
	}
#endif
#pragma endregion

#pragma region Gameplay Setup
	ctx.game.pongGame = new PongGame(ctx.system.viewportWidth, ctx.system.viewportHeight);

	ctx.engine.updateQueue.push_back(ctx.game.pongGame);
	ctx.engine.renderQueue.push_back(ctx.game.pongGame);
#pragma endregion

#pragma region Main Loop
	/*
	 * Just a couple of lines, here the program will
	 * spend 99% of its time.
	 * Here all game logic will hook and run, frame
	 * by frame.
	 */
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(MainLoop, 0, 1);
#else
	while(!ctx.engine.closeRequested)
		MainLoop();
#endif
#pragma endregion

#pragma region System Shutdown
	/*
	 * Not calling when building with emscripten,
	 * it follows a slightly different logic.
	 * The main loop started above with
	 *	emscripten_set_main_loop()
	 * schedules a call to the MainLoop function
	 * at intervals decided by the browser (but
	 * configured by us) but that function doesn't
	 * block execution so we don't want to shutdown
	 * the system before the first frame is
	 * processed.
	 * We'll call SystemShutdown() later, in the
	 * main loop.
	 */
#ifndef __EMSCRIPTEN__
	SystemShutdown();
#endif
#pragma endregion

	return 0;
}

int SystemSetup()
{
	//	Initialize SDL (here we can selectively initialize different modules using SDL_INIT_* OR'd constants)
	if(SDL_Init(SDL_INIT_MODE) < 0)
	{
		cout << "Couldn't initialize SDL2: " << SDL_GetError() << endl;
		return -1;
	}

	//	Check canvas size when targetting webgl
#ifndef __EMSCRIPTEN__
#ifndef _DEBUG
	SDL_DisplayMode displayMode;
	if(SDL_GetCurrentDisplayMode(0, &displayMode) == 0)
	{
		ctx.system.viewportWidth = displayMode.w;
		ctx.system.viewportHeight = displayMode.h;
	}
	else
#endif
	{
		ctx.system.viewportWidth = VIEWPORT_W;
		ctx.system.viewportHeight = VIEWPORT_H;
	}
#else
	emscripten_get_canvas_element_size(HTML_CANVAS_SELECTOR, &ctx.system.viewportWidth, &ctx.system.viewportHeight);
#endif

	//	Create the game window
	ctx.system.window = SDL_CreateWindow(
		"SDL Pong!",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		ctx.system.viewportWidth, ctx.system.viewportHeight,
		VIEWPORT_MODE
	);
	if(!ctx.system.window)
	{
		cout << "Couldn't create SDL window: " << SDL_GetError() << endl;
		return -1;
	}

	//	Get or create a rendeer for future render operations
	ctx.system.r = SDL_GetRenderer(ctx.system.window);
	if(!ctx.system.r)
	{
#ifdef _DEBUG
		cout << "Couldn't get SDL renderer from window: " << SDL_GetError() << endl;
		cout << "Trying to create a new renderer.." << endl;
#endif
		ctx.system.r = SDL_CreateRenderer(ctx.system.window, -1, SDL_RendererFlags::SDL_RENDERER_ACCELERATED);
		if(!ctx.system.r)
		{
			cout << "Couldn't create SDL renderer on window: " << SDL_GetError() << endl;
			return -1;
		}
#ifdef _DEBUG
		else
			cout << "Renderer created succesfully!" << endl;
#endif
	}
#ifdef _DEBUG
	else
		cout << "Renderer retrieved succesfully!" << endl;
#endif

	//	Initialize the TTF module
	if(TTF_Init() != 0)
	{
		cout << "Cannot initialize SDL_ttf: " << TTF_GetError() << endl;
		return -1;
	}
#ifdef _DEBUG
	else
		cout << "SDL_ttf intialized succesfully!" << endl;
#endif

	//	Initialize the IMG module
	int imgInitFlags = IMG_INIT_MODE;	//	Choose here what kind of files you want to support and take into account that you need to include spefici dll files
	int imgInitializedFormats = IMG_Init(imgInitFlags);
	if(imgInitFlags != imgInitializedFormats)
	{
		cout << "Couldn't fully initialize SDL_Image, expected " << imgInitFlags << " obtained " << imgInitializedFormats << ": " << IMG_GetError() << endl;
		return -1;
	}
#ifdef _DEBUG
	else
		cout << "SDL_Image initialized succesfully!" << endl;
#endif

	//	Initialize the Mixer module
	int mixerInitFlags = MIX_INIT_MODE;
	int mixerInitializedFormats = Mix_Init(mixerInitFlags);
	if(mixerInitFlags != mixerInitializedFormats)
	{
		cout << "Couldn't fully initialize SDL_Mixer, expected " << mixerInitFlags << " obtained " << mixerInitializedFormats << ": " << Mix_GetError() << endl;
		return -1;
	}
#ifdef _DEBUG
	else
		cout << "SDL_Mixer initialized succesfully!" << endl;
#endif
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
	{
		cout << "Couldn't open audio device: " << Mix_GetError() << endl;
		return -1;
	}
#ifdef _DEBUG
	else
		cout << "Audio device opened succesfully!" << endl;
#endif

	return 0;
}

void StartMusic()
{
	//	Load music from file
	string bgmFullPath = PathUtils::Combine(
		{
			SDL_GetBasePath(),
			"res",
			"sound",
			"bgm",
			PathUtils::AddMusicExtension("GameLoop_01")
		}
	);

	ctx.game.bgm = Mix_LoadMUS(bgmFullPath.c_str());

	//	Start music, if load succeeded
	if(ctx.game.bgm)
		Mix_FadeInMusic(ctx.game.bgm, -1, BGM_FADE_IN_TIME);
	else
		cout << "Couldn't load music from " << bgmFullPath << " [ERROR]: " << Mix_GetError() << endl;
}

void MainLoop()
{
	/*
	 * The main loop has the responsibility to mark the
	 * different stages of a frame (with "frame" we mean
	 * a unit of calculation that results in an image which
	 * will be presented to the player).
	 * Here's a breakdown of the stages:
	 * - FPS Regulation (pt1):
	 *		keep track of the current time to check how long
	 *		it took to calculate the entire frame and allows
	 *		for frame-time consistency at (pt2)
	 *		(this operation is not needed on all platforms)
	 * - Events/Input:
	 *		poll and consume all system events such as
	 *		hardware interrupts or window events and use
	 *		them to update the state of an Input class which
	 *		can be later used by the business logic
	 * - Update:
	 *		here an update message is broadcasted to all
	 *		game elements which subscribed to it, this is the
	 *		place where the game logic happens each frame and
	 *		where al modifications to the game state happen
	 * - Pre-Render:
	 *		an intermediate stage between update and render,
	 *		useful to make last changes after all the game
	 *		logic finished running (so in a consistent state),
	 *		it differs from update because subscriber have
	 *		access to the renderer here
	 * - Render:
	 *		a classic render loop, where the render target gets
	 *		cleared, all the subscribers get rendered and,
	 *		finally, the back and front buffers are swapped and
	 *		the final image is show to the player (during this
	 *		stage no modifications to the game state are allowed)
	 * - FPS Regulation (pt2):
	 *		based on data collected here and at (pt1), and
	 *		based on the target frame time, here it is calculated
	 *		a delay to wait to meet the target frame rate and
	 *		not overshoot it
	 *		(this operation is not needed on all platforms)
	 * - WebGL Shutdown:
	 *		a very specific stage, the reason for it is that
	 *		on webgl the main loop is handled outside of
	 *		the game code (the browser controls it) so here we
	 *		make specific operations that allow to shutdow the
	 *		application following the browser's logic
	 */
#pragma region Prepare FPS Regulation
	/*
	 * To keep a steady frame rate, we need to know
	 * how long it takes for a frame to make all
	 * calculations and render and then wait for the
	 * target frame time.
	 * To do so, when the frame starts, i.e. when
	 * entering the main 
	 * 
	 * When building for webgl, we let the browser decide
	 * the frame rate, which will typically match the
	 * monitor's refresh rate, so we're skipping all the
	 * framerate regulation stuff when targetting webgl.loop, we store the precise
	 * time.
	 */
#ifndef __EMSCRIPTEN__
	steady_clock::time_point frameStart = high_resolution_clock::now();
#endif
#pragma endregion

#pragma region Events/Input Loop
		/*
		 * Here we handle all the main events coming from
		 * both the OS and the input.
		 * NOTE: Here we receive all the events but we
		 * can filter what events are added to the queue
		 * from which SDL_PollEvent reads using a filter:
		 * see https://wiki.libsdl.org/SDL2/SDL_EventFilter
		 */
	SDL_Event currentEvent;
	while(SDL_PollEvent(&currentEvent))
	{
		switch(currentEvent.type)
		{
#ifndef __EMSCRIPTEN__
				/*
				 * Not handling quit event when targetting
				 * webgl, for a more platform-specific UX
				 */
			case SDL_EventType::SDL_QUIT:
				ctx.engine.closeRequested = true;	//	When the red X in the top right corner of the window is clicked, let's commit a quit
				break;
#endif
			case SDL_EventType::SDL_KEYDOWN:
				Input::Get().NotifyKeyDown(currentEvent.key.keysym.sym);
#ifndef __EMSCRIPTEN__
					/*
					 * Not handling escape button when targetting
					 * webgl, for a more platform-specific UX
					 */
				if(currentEvent.key.keysym.sym == SDLK_ESCAPE)
					ctx.engine.closeRequested = true;	//	Let's use the Escape button to quit the game
#endif
				break;
			case SDL_EventType::SDL_KEYUP:
				Input::Get().NotifyKeyUp(currentEvent.key.keysym.sym);
				break;
		}
	}
#pragma endregion

#pragma region Update Loop (Logic)
	for(IUpdatable *& updatable : ctx.engine.updateQueue)
		updatable->Update();
#pragma endregion

#pragma region Render Loop
	//	Send a pre-render message to all subscribers so they can prepare for rendering
	for(IRenderable * const & renderable : ctx.engine.renderQueue)
		renderable->PreRender(ctx.system.r);

	//	Let's clear the canvas before drawing a new frame
	SDL_SetRenderDrawColor(ctx.system.r, RENDER_CLEAR_COLOR);
	SDL_RenderClear(ctx.system.r);

	//	Draw all renderables to the back buffer (Render is a const function)
	for(const IRenderable * const & renderable : ctx.engine.renderQueue)
		renderable->Render(ctx.system.r);

	//	Swap front and back buffer to show results of the render
	SDL_RenderPresent(ctx.system.r);
#pragma endregion

#pragma region FPS Regulation
	/*
	 * We calculate the frame time, relative to the frame start time.
	 * If it's below the target frame time, we'll wait for the difference.
	 * If, instead, the frame take longer than the frame time we have two
	 * roads to walk:
	 * - variable frame time: we just don't wait and rush into the next frame
	 * - fixed frame time: we skip a frame to align to the next fixed frame time
	 *
	 * Important note: when working with high resolution clock (or numbers
	 * in general), data is usually stored in large data formats such as
	 * long int or even long long int, so it's good practice to avoid
	 * implicit conversions to make 100% clear what type of data we're
	 * carying around.
	 * 
	 * As stated above, FPS regulation is entrusted to the browser for
	 * webgl builds, so we'll skip the manual frame rate regulation here
	 * too.
	 */
#ifndef __EMSCRIPTEN__
	long long elapsedMillis = duration_cast<milliseconds>(high_resolution_clock::now() - frameStart).count();
#ifdef FRAME_SKIP
	elapsedMillis %= TARGET_FPS;
#endif
	long long waitMillis = (1000 / TARGET_FPS) - elapsedMillis;
	if(waitMillis > 0)
		SDL_Delay((int)waitMillis);
#endif
#pragma endregion

#pragma region WebGL Shutdown
	/*
	 * When targetting webgl this function (MainLoop) is
	 * called again and again by the browser. When the
	 * game requests a close, we need to shutdown the
	 * system.
	 */
#ifdef __EMSCRIPTEN__
	if(ctx.engine.closeRequested)
		SystemShutdown();
#endif
#pragma endregion
}

void StopMusic()
{
	//	Stop playing music
	if(Mix_PlayingMusic())
	{
#ifdef __EMSCRIPTEN__
		Mix_HaltMusic();
#else
		Mix_FadeOutMusic(BGM_FADE_OUT_TIME);
		SDL_Delay(BGM_FADE_OUT_TIME);
#endif
	}

	//	Free music and reset pointer
	if(ctx.game.bgm)
	{
		Mix_FreeMusic(ctx.game.bgm);
		ctx.game.bgm = nullptr;
	}
}

void SystemShutdown()
{
	/*
	 * Closing operations are very much important.
	 * Always remember to clean all created objects
	 * and to quit all the systems.
	 * This is especially useful when the same
	 * application opens and closes the resources
	 * multiple times in the same run.
	 *
	 * Specific for emscripten, we stop the
	 * main loop.
	 */
#ifdef __EMSCRIPTEN__
	emscripten_cancel_main_loop();
#endif

	//	Stop playing the BGM
	StopMusic();

	//	Dispose the game
	if(ctx.game.pongGame)
	{
		delete ctx.game.pongGame;
		ctx.game.pongGame = nullptr;
	}

	//	Quit all systems
	SDL_DestroyWindow(ctx.system.window);
	Mix_CloseAudio();
	Mix_Quit();
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}
