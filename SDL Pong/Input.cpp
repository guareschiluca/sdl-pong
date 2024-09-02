#include "Input.h"

void Input::NotifyKeyDown(const SDL_Keycode & key)
{
	buttonsMap[key] = true;
}

void Input::NotifyKeyUp(const SDL_Keycode & key)
{
	buttonsMap[key] = false;
}

bool Input::GetKey(const SDL_Keycode & key) const
{
	auto keyCheck = buttonsMap.find(key);

	return keyCheck != buttonsMap.end() && keyCheck->second;
}
