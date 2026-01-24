#version 330 core

in vec3 norm;
in vec3 fragPos;
in vec2 texCoords;

out vec4 fragColor;

uniform vec3 viewPos;

struct Material
{
	sampler2D diffuse;
	float specular;
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
//uniform SimpleLight simpleLight;

struct DirectionalLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
//uniform DirectionalLight directionalLight;

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
uniform PointLight pointLight;

struct SpotLight
{
	vec3 position;
	vec3 direction;
	float cutoff;
	float outerCutoff;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
//uniform SpotLight spotLight;

vec3 calculatePointLight(vec3 normalizedNorm, vec3 normalizedViewDir)
{
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
	vec3 specularColor = vec3(material.specular);

	vec3 normalizedLightDir = normalize(pointLight.position - fragPos);
	vec3 normalizedHalfDir = normalize(normalizedLightDir + normalizedViewDir);

	float distance = length(pointLight.position - fragPos);
	float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * distance * distance);

	vec3 ambient = diffuseColor * pointLight.ambient;

	float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseAmount * diffuseColor * pointLight.diffuse;

	float specularAmount = pow(max(dot(normalizedNorm, normalizedHalfDir), 0.0f), material.shininess);
	vec3 specular = specularAmount * specularColor * pointLight.specular;

	vec3 result = (ambient + diffuse + specular) * attenuation;
	return result;
}

void main()
{
	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedViewDir = normalize(viewPos - fragPos);

	vec3 result = vec3(0.0f);
	result += calculatePointLight(normalizedNorm, normalizedViewDir);

	fragColor = vec4(result, 1.0f);
	//fragColor = vec4(0.5f, 0.0f, 1.0f, 1.0f);
}