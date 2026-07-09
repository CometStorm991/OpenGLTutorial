#version 330 core

layout(location = 0) out vec4 fragColor;

in vec3 localPos;

uniform samplerCube environmentMap;
uniform float roughness;
uniform float resolution;

const float pi = 3.14159265359f;
// From LearnOpenGL
float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;
	
    return num / denom;
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness * roughness;
	
	float phi = 2.0f * pi * Xi.x;
	float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	vec3 up = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

const uint sampleCount = 1024u;

void main()
{
	vec3 N = normalize(localPos);
	vec3 V = N;

	float totalWeight = 0.0f;
	vec3 prefilteredColor = vec3(0.0f);
	for (uint i = 0u; i < sampleCount; i++)
	{
		vec2 Xi = Hammersley(i, sampleCount);
		vec3 H = importanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.0f * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0f);
		if (NdotL > 0.0f)
		{
			float HdotV = max(dot(H, V), 0.0f);
			float NdotH = max(dot(N, H), 0.0f);
			float D = DistributionGGX(N, H, roughness);
			float pdf = (D * NdotH / (4.0f * HdotV)) + 0.0001f;

			float saTexel  = 4.0f * pi / (6.0f * resolution * resolution);
			float saSample = 1.0f / (float(sampleCount) * pdf + 0.0001f);
			float mipLevel = roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);

			prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;

	fragColor = vec4(prefilteredColor, 1.0f);
}