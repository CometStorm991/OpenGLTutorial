#version 330 core

in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

void main()
{
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lightColor;

	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedLightDir = normalize(lightPos - fragPos);

	float diffuseStrength = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseStrength * lightColor;
	
	vec3 result = (ambient + diffuse) * objectColor;
	fragColor = vec4(result, 1.0f);
}