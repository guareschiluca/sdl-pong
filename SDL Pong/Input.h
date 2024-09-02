#pragma once

#pragma region C++ Includes
#include <map>
#pragma endregion

#pragma region SDL Includes
#include "SDL_keycode.h"
#pragma endregion

using namespace std;

class Input
{
	// Fields
public:
protected:
private:
	map<SDL_Keycode, bool> buttonsMap;
	// Constructors
public:
	// Delete copy constructor and assignment operator (singleton protection)
	Input(const Input &) = delete;
	Input & operator=(const Input &) = delete;
protected:
private:
	Input() { }
	// Methods
public:
	static Input & Get()
	{
		//	Singleton implementation
		static Input instance;
		return instance;
	}
	void NotifyKeyDown(const SDL_Keycode & key);
	void NotifyKeyUp(const SDL_Keycode & key);

	bool GetKey(const SDL_Keycode & key) const;
protected:
private:
};

