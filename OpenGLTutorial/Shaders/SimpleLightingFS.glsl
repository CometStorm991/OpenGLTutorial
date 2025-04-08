#version 330 core

in vec3 norm;
in vec3 fragPos;
in vec2 texCoords;

out vec4 fragColor;

uniform vec3 viewPos;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

struct SimpleLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform SimpleLight simpleLight;

vec3 calculateSimpleLight(vec3 normalizedNorm, vec3 normalizedViewDir)
{
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
	vec3 specularColor = vec3(texture(material.specular, texCoords));

	vec3 normalizedLightDir = normalize(simpleLight.position - fragPos);

	vec3 ambient = diffuseColor * simpleLight.ambient;

	float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseAmount * diffuseColor * simpleLight.diffuse;

	vec3 reflectDir = reflect(-normalizedLightDir, normalizedNorm);
	float specularAmount = pow(max(dot(normalizedViewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = specularAmount * specularColor * simpleLight.specular;

	vec3 result = ambient + diffuse + specular;
	return result;
}

void main()
{
	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedViewDir = normalize(viewPos - fragPos);

	vec3 result = calculateSimpleLight(normalizedNorm, normalizedViewDir);

	fragColor = vec4(result, 1.0f);
}