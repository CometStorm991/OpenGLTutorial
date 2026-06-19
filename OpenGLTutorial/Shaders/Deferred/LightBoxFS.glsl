#version 460 core

flat in int vInstanceId;

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
	fragColor = vec4(lights[vInstanceId].diffuse, 1.0f);
	//fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}