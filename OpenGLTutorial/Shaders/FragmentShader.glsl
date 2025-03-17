#version 330 core

layout(location = 0) out vec4 color;

uniform float redColor;

void main() {
	color = vec4(redColor, 0.0f, 1.0f, 0.0f);
}