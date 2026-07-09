#version 330 core

layout(location = 0) in vec2 aPos;

out vec2 localPos;

void main()
{
	gl_Position = vec4(aPos, 0.0f, 1.0f);
	localPos = aPos * 0.5f + 0.5f;
}