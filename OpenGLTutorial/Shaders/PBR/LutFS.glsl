#version 330 core

layout(location = 0) out vec4 fragColor;

in vec2 localPos;

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
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float k = (roughness * roughness) / 2.0f;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
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

vec2 integrateBRDF(float NdotV, float roughness)
{
	vec3 V = vec3(sqrt(1.0f - NdotV * NdotV), 0.0f, NdotV);

	float a = 0.0f;
	float b = 0.0f;

	vec3 N = vec3(0.0f, 0.0f, 1.0f);
	
	for (uint i = 0u; i < sampleCount; i++)
	{
		vec2 Xi = Hammersley(i, sampleCount);
		vec3 H = importanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.0f * dot(V, H) * H - V);

		// Both NdotL and NdotH are projections onto vec3(0.0f, 0.0f, 1.0f), so this trick works
		float NdotL = max(L.z, 0.0f);
		float NdotH = max(H.z, 0.0f);
		float VdotH = max(dot(V, H), 0.0f);

		if (NdotL > 0.0f)
		{
			float G = GeometrySmith(N, V, L, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0f - VdotH, 5.0f);

			a += (1.0f - Fc) * G_Vis;
			b += Fc * G_Vis;
		}
	}

	a /= float(sampleCount);
	b /= float(sampleCount);
	
	return vec2(a, b);
}

void main()
{
	vec2 integratedBRDF = integrateBRDF(localPos.x, localPos.y);
	fragColor = vec4(integratedBRDF, 0.0f, 1.0f);
	//fragColor = vec4(localPos.x, localPos.y, 1.0f, 1.0f);
}