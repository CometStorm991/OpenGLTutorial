#version 330 core

in vec3 norm;
in vec3 fragPos;
in vec2 texCoords;
in vec4 fragPosLightSpace;

out vec4 fragColor;

uniform vec3 viewPos;
uniform sampler2D depthMap;

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

float calculateShadow(vec4 fragPosLightSpace);

vec3 calculatePointLight(vec3 normalizedNorm, vec3 normalizedViewDir)
{
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
	vec3 specularColor = vec3(texture(material.specular, texCoords));

	vec3 normalizedLightDir = normalize(pointLight.position - fragPos);

	float distance = length(pointLight.position - fragPos);
	float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * distance * distance);

	vec3 ambient = diffuseColor * pointLight.ambient;

	float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseAmount * diffuseColor * pointLight.diffuse;

	vec3 reflectDir = reflect(-normalizedLightDir, normalizedNorm);
	float specularAmount = pow(max(dot(normalizedViewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = specularAmount * specularColor * pointLight.specular;

	float shadow = calculateShadow(fragPosLightSpace);
	vec3 result = (ambient + (1.0f - shadow) * (diffuse + specular)) * attenuation;
	return result;
}

float calculateShadow(vec4 lightSpace)
{
	vec3 projCoords = lightSpace.xyz / lightSpace.w;
	projCoords = projCoords * 0.5f + 0.5f;
	float closestDepth = texture(depthMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

	return shadow;
}

void main()
{
	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedViewDir = normalize(viewPos - fragPos);

	vec3 result = vec3(0.0f);
	result += calculatePointLight(normalizedNorm, normalizedViewDir);
	//vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	//projCoords = projCoords * 0.5f + 0.5f;
	//result += vec3(projCoords.x > 0.35f ? 1.0f : 0.0f, 0.0f, 0.0f);
	//result += vec3(calculateShadow(fragPosLightSpace));

	fragColor = vec4(result, 1.0f);
}