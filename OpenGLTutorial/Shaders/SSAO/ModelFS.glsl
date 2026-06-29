#version 460 core

in vec3 norm;
in vec3 fragPos;
in vec2 texCoords;

in vec3 ssaoFragPos;
in vec3 ssaoNorm;

layout (location = 0) out vec3 gPos;
layout (location = 1) out vec3 gNorm;
layout (location = 2) out vec4 gDiffSpec;
layout (location = 3) out vec3 gSSAOPos;
layout (location = 4) out vec3 gSSAONorm;

uniform vec3 viewPos;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

void main()
{
	gPos = fragPos;
	vec3 normalizedNorm = normalize(norm);
	gNorm = normalizedNorm;

	gDiffSpec.rgb = texture(material.diffuse, texCoords).rgb;
	//gDiffSpec.a = texture(material.specular, texCoords).r;

	gSSAOPos = ssaoFragPos;
	gSSAONorm = normalize(ssaoNorm);
}