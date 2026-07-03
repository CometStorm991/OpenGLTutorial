#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTan;
layout(location = 4) in mat4 aModel;

uniform mat4 view;
uniform mat4 projection;

out vec2 vFragPos;

void main()
{
	gl_Position = projection * view * aModel * vec4(aPos, 1.0f);


}