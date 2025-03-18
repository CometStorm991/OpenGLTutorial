#version 330 core

in vec2 textureCoordinates;

layout(location = 0) out vec4 color;

uniform float redColor;
uniform sampler2D inputTexture;

void main() {
	// color = vec4(textureCoordinates.x, textureCoordinates.y, 1.0f, 1.0f);
	color = texture(inputTexture, textureCoordinates);
}