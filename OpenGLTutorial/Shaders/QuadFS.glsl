#version 330 core

layout(location = 0) out vec4 color;

in vec2 textureCoords;

uniform sampler2D screenTexture;

void main()
{
	color = texture(screenTexture, textureCoords);
    // if (color == vec4(0.0f, 0.0f, 0.0f, 1.0f))
    // {
    //     color = vec4(textureCoords, 1.0f, 1.0f);
    // }
}