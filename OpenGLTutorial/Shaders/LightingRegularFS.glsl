#version 330 core

in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lightColor;

	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedLightDir = normalize(lightPos - fragPos);

	float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseAmount * lightColor;

	float specularStrength = 0.5f;
	vec3 normalizedViewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-normalizedViewDir, normalizedNorm);
	float specularAmount = pow(max(dot(normalizedViewDir, reflectDir), 0.0f), 32.0f);
	vec3 specular = specularStrength * specularAmount * lightColor;
	
	vec3 result = (ambient + diffuse + specular) * objectColor;
	fragColor = vec4(result, 1.0f);
}