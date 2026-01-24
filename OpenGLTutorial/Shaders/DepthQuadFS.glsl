#version 330 core

layout(location = 0) out vec4 color;

in vec2 textureCoords;

uniform sampler2D screenTexture;

void main()
{
	float depthVal = texture(screenTexture, textureCoords).r;
    color = vec4(vec3(depthVal), 1.0f);
}