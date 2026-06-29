#version 460 core

in vec2 vTexCoords;

out float fragColor;

uniform sampler2D posSamp;
uniform sampler2D normSamp;
uniform sampler2D texNoiseSamp;

uniform vec3 samples[64];
uniform int sampleCount;
uniform mat4 projection;
uniform vec2 noiseScale;

void main()
{
	vec3 fragPos = texture(posSamp, vTexCoords).xyz;
	vec3 normal = texture(normSamp, vTexCoords).xyz;
	vec3 randomVec = normalize(texture(texNoiseSamp, vTexCoords * noiseScale).xyz);

	// Gram-Schmidt process - creating orthonormal basis
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	float occlusion = 0.0f;
	float radius = 1.0f;
	float bias = 0.025f;
	for (uint i = 0; i < sampleCount; i++)
	{
		vec3 samplePos = tbn * samples[i];
		samplePos = fragPos + samplePos * radius;

		vec4 offset = vec4(samplePos, 1.0f);
		offset      = projection * offset;
		offset.xyz /= offset.w;
		offset.xyz  = offset.xyz * 0.5f + 0.5f;

		float sampleDepth = texture(posSamp, offset.xy).z;
		float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0f : 0.0f) * rangeCheck;
	}
	occlusion = 1.0f - (occlusion / sampleCount);
	fragColor = occlusion;
}