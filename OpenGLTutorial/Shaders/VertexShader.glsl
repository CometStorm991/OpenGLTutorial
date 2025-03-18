#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 textureCoordinateInput;

out vec2 textureCoordinates;

void main() {
	gl_Position = position;

	textureCoordinates = textureCoordinateInput;
}