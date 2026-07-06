#version 330 core

layout(location = 0) out vec4 fragColor;

in vec3 localPos;

uniform sampler2D equirectMap;

const vec2 invAtan = vec2(0.1591f, 0.3183f);
vec2 sampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5f;
	return uv;
}

void main()
{
	vec2 uv = sampleSphericalMap(normalize(localPos));
	vec3 color = texture(equirectMap, uv).rgb;

	fragColor = vec4(color, 1.0f);
}