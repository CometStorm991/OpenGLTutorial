#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 3) in mat4 aModel;
layout(location = 7) in vec3 aColor;

out vec3 color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	color = aColor;
	gl_Position = projection * view * aModel * vec4(aPos, 1.0f);
}