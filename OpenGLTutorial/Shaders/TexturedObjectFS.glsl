#version 460 core

in vec3 norm;
in vec3 fragPos;
in vec2 texCoords;

out vec4 fragColor;

uniform vec3 viewPos;
uniform samplerCubeArray depthCubemaps;
uniform float farPlane;

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
layout(std430, binding = 0) buffer LightsBuf
{
	PointLight lights[];
};

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

float calculateShadow(vec3 fragPos, int lightIndex);

vec3 calculatePointLight(vec3 normalizedNorm, vec3 normalizedViewDir)
{
	vec3 result = vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < lights.length(); i++)
	{
		vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
		vec3 specularColor = vec3(material.specular);

		vec3 normalizedLightDir = normalize(lights[i].position - fragPos);
		vec3 normalizedHalfDir = normalize(normalizedLightDir + normalizedViewDir);

		float distance = length(lights[i].position - fragPos);
		float attenuation = 1.0f / (lights[i].constant + lights[i].linear * distance + lights[i].quadratic * distance * distance);

		vec3 ambient = diffuseColor * lights[i].ambient;

		float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
		vec3 diffuse = diffuseAmount * diffuseColor * lights[i].diffuse;

		float specularAmount = pow(max(dot(normalizedNorm, normalizedHalfDir), 0.0f), material.shininess);
		vec3 specular = specularAmount * specularColor * lights[i].specular;

		float shadow = calculateShadow(fragPos, i);

		result += (ambient + (1.0f - shadow) * (diffuse + specular)) * attenuation;
	}

	return result;
}

float calculateShadow(vec3 fragPos, int lightIndex)
{
	vec3 fragToLight = fragPos - lights[lightIndex].position;
	float closestDepth = texture(depthCubemaps, vec4(fragToLight, lightIndex)).r;
	closestDepth *= farPlane;
	float currentDepth = length(fragToLight);
	
	float bias = 0.05f;
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

	return shadow;
}

void main()
{
	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedViewDir = normalize(viewPos - fragPos);

	vec3 result = vec3(0.0f);
	result += calculatePointLight(normalizedNorm, normalizedViewDir);

	fragColor = vec4(result, 1.0f);
}