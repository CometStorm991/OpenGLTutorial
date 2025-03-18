#version 330 core

in vec2 textureCoordinates;

layout(location = 0) out vec4 color;

uniform float redColor;

uniform sampler2D inputTexture0;
uniform sampler2D inputTexture1;

void main() {
	vec4 gradientColor = vec4(textureCoordinates.x, textureCoordinates.y, 1.0f, 1.0f);
	vec4 texture0Color = texture(inputTexture0, textureCoordinates);
	vec4 texture1Color = texture(inputTexture1, textureCoordinates);
	color = mix(texture0Color * gradientColor, texture1Color, 0.5);
}