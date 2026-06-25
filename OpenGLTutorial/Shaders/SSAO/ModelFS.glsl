#version 460 core

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

vec3 calculatePointLight(vec3 normalizedNorm, vec3 normalizedViewDir)
{
	vec3 result = vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < lights.length(); i++)
	{
		vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
		//vec3 specularColor = vec3(texture(material.diffuse, texCoords));

		vec3 normalizedLightDir = normalize(lights[i].position - fragPos);
		vec3 normalizedHalfDir = normalize(normalizedLightDir + normalizedViewDir);

		float distance = length(lights[i].position - fragPos);
		float attenuation = 1.0f / (lights[i].constant + lights[i].linear * distance + lights[i].quadratic * distance * distance);

		vec3 ambient = diffuseColor * lights[i].ambient;

		float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
		vec3 diffuse = diffuseAmount * diffuseColor * lights[i].diffuse;

		//float specularAmount = pow(max(dot(normalizedNorm, normalizedHalfDir), 0.0f), material.shininess);
		//vec3 specular = specularAmount * specularColor * lights[i].specular;

		//result += (ambient + diffuse + specular) * attenuation;
		result += (ambient + diffuse) * attenuation;
	}

	return result;
}

void main()
{
	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedViewDir = normalize(viewPos - fragPos);
	
	vec3 result = vec3(0.0f);
	// result += calculateDirectionalLight(normalizedNorm, normalizedViewDir);
	result += calculatePointLight(normalizedNorm, normalizedViewDir);
	
	fragColor = vec4(result, 1.0f);
	//fragColor = vec4(0.5f, 0.0f, 1.0f, 1.0f);
}