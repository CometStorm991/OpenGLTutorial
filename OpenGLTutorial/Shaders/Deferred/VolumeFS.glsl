#version 460 core

flat in int vInstanceId;

out vec4 fragColor;

uniform sampler2D posSamp;
uniform sampler2D normSamp;
uniform sampler2D albedoSpecSamp;

uniform vec3 viewPos;
uniform float materialShininess;
uniform vec2 screenDims;

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

void main()
{
	vec2 screenFragPos = gl_FragCoord.xy / screenDims.xy;

	vec3 diffuseColor = texture(albedoSpecSamp, screenFragPos).rgb;
	vec3 specularColor = vec3(texture(albedoSpecSamp, screenFragPos).a);

	vec3 fragPos = texture(posSamp, screenFragPos).rgb;
	vec3 norm = texture(normSamp, screenFragPos).rgb;

	vec3 normalizedNorm = normalize(norm);
	vec3 normalizedViewDir = normalize(viewPos - fragPos);

	vec3 normalizedLightDir = normalize(lights[vInstanceId].position - fragPos);
	vec3 normalizedHalfDir = normalize(normalizedLightDir + normalizedViewDir);

	float distance = length(lights[vInstanceId].position - fragPos);
	float attenuation = 1.0f / (lights[vInstanceId].constant + lights[vInstanceId].linear * distance + lights[vInstanceId].quadratic * distance * distance);

	vec3 ambient = diffuseColor * lights[vInstanceId].ambient;

	float diffuseAmount = max(dot(normalizedNorm, normalizedLightDir), 0.0f);
	vec3 diffuse = diffuseAmount * diffuseColor * lights[vInstanceId].diffuse;

	float specularAmount = pow(max(dot(normalizedNorm, normalizedHalfDir), 0.0f), materialShininess);
	vec3 specular = specularAmount * specularColor * lights[vInstanceId].specular;

	fragColor = vec4((ambient + diffuse + specular) * attenuation, 1.0f);
	if (fragColor == vec4(0.0f, 0.0f, 0.0f, 1.0f) || fragColor == vec4(0.0f, 0.0f, 0.0f, 0.0f))
    {
        //fragColor = vec4(screenFragPos, 1.0f, 1.0f);
    }
	//fragColor = vec4((ambient + diffuse + specular), 1.0f);
}