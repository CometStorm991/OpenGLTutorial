#version 330 core

in vec3 fragPos;
in vec3 norm;

out vec4 fragColor;

uniform vec3 viewPos;
uniform samplerCube skybox;

struct Metal
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;

	float reflectance;
};
uniform Metal metal;

struct DirectionalLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirectionalLight directionalLight;

vec3 calculateDirectionalLight(vec3 normalizedNorm, vec3 normalizedViewDir)
{
	vec3 diffuseColor = metal.diffuse;
	vec3 specularColor = metal.specular;

	vec3 normalizedLightDir = normalize(-directionalLight.direction);

	vec3 ambient = diffuseColor * directionalLight.ambient * metal.ambient;

	float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseAmount * diffuseColor * directionalLight.diffuse;

	vec3 reflectDir = reflect(-normalizedLightDir, normalizedNorm);
	float specularAmount = pow(max(dot(normalizedViewDir, reflectDir), 0.0f), metal.shininess);
	vec3 specular = specularAmount * specularColor * directionalLight.specular;

	vec3 result = ambient + diffuse + specular;
	return result;
}

void main()
{
	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedViewDir = normalize(viewPos - fragPos);

	vec3 result = vec3(0.0f);
	result += calculateDirectionalLight(normalizedNorm, normalizedViewDir) * (1.0 - metal.reflectance);

	vec3 reflectDir = reflect(-normalizedViewDir, normalizedNorm);
	vec3 reflectColor = texture(skybox, reflectDir).rgb;
	result += reflectColor * metal.reflectance;

	fragColor = vec4(result, 1.0f);
	//fragColor = vec4(0.5f, 0.0f, 0.5f, 1.0f);
}