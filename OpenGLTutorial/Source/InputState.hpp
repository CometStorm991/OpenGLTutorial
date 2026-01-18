#pragma once

struct InputState
{
	float lastX = 0.0f;
	float lastY = 0.0f;
	float posX = 0.0f;
	float posY = 0.0f;

	bool w = false;
	bool s = false;
	bool d = false;
	bool a = false;
	bool q = false;
	bool e = false;
};