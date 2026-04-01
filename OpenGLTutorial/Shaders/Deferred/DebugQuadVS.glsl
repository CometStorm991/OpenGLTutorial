#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec2 aOffset;
layout(location = 3) in float aTexSampIndex;

out vec2 textureCoords;
flat out uint texSampIndex;

void main()
{
	gl_Position = vec4(aPos + aOffset, 0.0f, 1.0f);
	textureCoords = aTexCoords;
	texSampIndex = uint(aTexSampIndex);
}