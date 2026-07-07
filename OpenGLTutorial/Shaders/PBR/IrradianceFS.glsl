#version 330 core

layout(location = 0) out vec4 fragColor;

in vec3 localPos;

uniform samplerCube environmentMap;

const float pi = 3.14159265359f;

void main()
{
	vec3 normal = normalize(localPos);

	vec3 irradiance = vec3(0.0f);

	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 right = normalize(cross(up, normal));
	up = normalize(cross(normal, right));

	float sampleDelta = 0.010f;
	float sampleCount = 0.0f;

	for (float phi = 0.0f; phi < 2.0f * pi; phi += sampleDelta)
	{
		for (float theta = 0.0f; theta < 0.5f * pi; theta += sampleDelta)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			sampleCount++;
		}
	}

	irradiance = pi * irradiance * (1.0f / sampleCount);

	fragColor = vec4(irradiance, 1.0f);
}