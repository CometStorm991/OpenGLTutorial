#version 330 core

layout(location = 0) out vec4 color;

in vec2 textureCoords;

uniform sampler2D screenTexture;
uniform float exposure;

void main()
{
	vec3 hdrColor = texture(screenTexture, textureCoords).rgb;

	vec3 mapped = vec3(1.0f) - exp(-hdrColor * exposure);

	color = vec4(mapped, 1.0f);
}