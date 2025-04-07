#version 330 core

in vec3 norm;
in vec3 fragPos;
in vec2 texCoords;

out vec4 fragColor;

uniform vec3 viewPos;

struct Material
{
	sampler2D diffuse;
	vec3 specular;
	float shininess;
};
uniform Material material;

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform Light light;

void main()
{
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));

	vec3 ambient = diffuseColor * light.ambient;

	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedLightDir = normalize(light.position - fragPos);

	float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseAmount * diffuseColor * light.diffuse;

	vec3 normalizedViewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-normalizedLightDir, normalizedNorm);
	float specularAmount = pow(max(dot(normalizedViewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = specularAmount * material.specular * light.specular; 
	
	vec3 result = ambient + diffuse + specular;
	fragColor = vec4(result, 1.0f);
}