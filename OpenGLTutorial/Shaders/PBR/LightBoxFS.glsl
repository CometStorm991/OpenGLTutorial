#version 460 core

flat in int vInstanceId;
in vec3 vNorm;

out vec4 fragColor;

struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
layout(std430, binding = 0) buffer LightsBuf
{
	PointLight lights[];
};

void main()
{
	vec3 posNorm = vNorm * 0.5f + 0.5f;
	vec3 avg = lights[vInstanceId].diffuse * 0.7f + posNorm * 0.3f;
	fragColor = vec4(avg * 5.0f, 1.0f);
}