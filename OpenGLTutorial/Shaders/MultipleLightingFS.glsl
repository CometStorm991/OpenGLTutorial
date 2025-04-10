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

struct DirectionalLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirectionalLight directionalLight;

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
uniform SpotLight spotLight;

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

vec3 calculateDirectionalLight(vec3 normalizedNorm, vec3 normalizedViewDir)
{
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
	vec3 specularColor = vec3(texture(material.specular, texCoords));

	vec3 normalizedLightDir = normalize(-directionalLight.direction);

	vec3 ambient = diffuseColor * directionalLight.ambient;

	float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseAmount * diffuseColor * directionalLight.diffuse;

	vec3 reflectDir = reflect(-normalizedLightDir, normalizedNorm);
	float specularAmount = pow(max(dot(normalizedViewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = specularAmount * specularColor * directionalLight.specular;

	vec3 result = ambient + diffuse + specular;
	return result;
}

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

	vec3 result = (ambient + diffuse + specular) * attenuation;
	return result;
}

vec3 calculateSpotLight(vec3 normalizedNorm, vec3 normalizedViewDir)
{
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
	vec3 specularColor = vec3(texture(material.specular, texCoords));

	vec3 normalizedLightDir = normalize(spotLight.position - fragPos);

	float distance = length(spotLight.position - fragPos);
	float attenuation = 1.0 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * distance * distance);

	vec3 ambient = diffuseColor * spotLight.ambient;

	float cosTheta = dot(normalizedLightDir, normalize(-spotLight.direction));
	float intensity = clamp((cosTheta - spotLight.cutoff) / (spotLight.cutoff - spotLight.outerCutoff), 0.0f, 1.0f);

	float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseAmount * diffuseColor * spotLight.diffuse * intensity;

	vec3 reflectDir = reflect(-normalizedLightDir, normalizedNorm);
	float specularAmount = pow(max(dot(normalizedViewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = specularAmount * specularColor * spotLight.specular * intensity;

	vec3 result = (ambient + diffuse + specular) * attenuation;
	return result;
}

void main()
{
	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedViewDir = normalize(viewPos - fragPos);

	vec3 result = vec3(0.0f);
	// result += calculateDirectionalLight(normalizedNorm, normalizedViewDir);
	result += calculatePointLight(normalizedNorm, normalizedViewDir);
	result += calculateSpotLight(normalizedNorm, normalizedViewDir);

	fragColor = vec4(result, 1.0f);
}