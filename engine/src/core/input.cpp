#include "pch.h"
#include "core/input.h"

using namespace SC;

namespace
{
	std::array<bool, KEY_LAST> m_keyDown;
	std::array<uint8_t, KEY_LAST> m_keyStates;

	float mouseX{0}, mouseY{0};
}

bool Input::IsKeyDown(int keyCode)
{
	if (keyCode < 0 && keyCode > KEY_LAST)
		return false;

	if (m_keyStates.at(keyCode) == KeyStatePressed || m_keyStates.at(keyCode) == KeyStateHeld)
		return true;

	return false;
}

bool Input::IsKeyPressed(int keyCode)
{
	if (keyCode < 0 && keyCode > KEY_LAST)
		return false;

	if (m_keyStates.at(keyCode) == KeyStatePressed)
		return true;

	return false;
}

bool Input::IsKeyReleased(int keyCode)
{
	if (keyCode < 0 && keyCode > KEY_LAST)
		return false;

	if (m_keyStates.at(keyCode) == KeyStateReleased)
		return true;

	return false;
}

void Input::SetKeyDown(int keyCode, bool down)
{
	if (keyCode < 0 && keyCode > KEY_LAST)
		return;

	m_keyDown.at(keyCode) = down;
}

void Input::Update()
{
	for (int i = 0; i < m_keyDown.size(); ++i)
	{
		uint8_t& state = m_keyStates[i];

		if (m_keyDown[i])
		{
			if (state == KeyStateNone)
				state = KeyStatePressed;
			else if (state == KeyStatePressed)
				state = KeyStateHeld;
		}
		else
		{
			if (state == KeyStatePressed || state == KeyStateHeld)
				state = KeyStateReleased;
			else if (state == KeyStateReleased)
				state = KeyStateNone;
		}
	}
}

void Input::SetMousePos(float x, float y)
{
	mouseX = x;
	mouseY = y;
}

void Input::GetMousePos(float& x, float& y)
{
	x = mouseX;
	y = mouseY;
}
