#version 460 core

in vec3 vFragPos;
in vec3 vNorm;
in vec3 vTang;
in vec3 vBi;
in vec2 vTexCoords;

out vec4 fragColor;

uniform sampler2D albedoSamp;
uniform sampler2D normSamp;
uniform sampler2D metallicSamp;
uniform sampler2D roughnessSamp;

uniform samplerCube irradianceSamp;

uniform vec3 viewPos;

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

// From LearnOpenGL
const float pi = 3.14159265359f;
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

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

void main()
{
	vec3 albedo = texture(albedoSamp, vTexCoords).xyz;
	float metallic = texture(metallicSamp, vTexCoords).x;
	float roughness = texture(roughnessSamp, vTexCoords).x;
	float ao = 1.0f;

	mat3 tbn = mat3(normalize(vTang), normalize(vBi), normalize(vNorm));

	vec3 normMap = texture(normSamp, vTexCoords).xyz;
	normMap = normMap * 2.0f - 1.0f;
	vec3 norm = normalize(tbn * normMap);
	
	vec3 viewDir = normalize(viewPos - vFragPos);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, metallic);

	vec3 l0 = vec3(0.0f);
	for (uint i = 0; i < lights.length(); i++)
	{
		vec3 lightDir = normalize(lights[i].position - vFragPos);
		vec3 halfDir = normalize(lightDir + viewDir);

		// radiance
		float dist = length(lights[i].position - vFragPos);
		float attenuation = 1.0f / (dist * dist);
		vec3 radiance = lights[i].diffuse;

		// Cook-Torrance
		float NDF = DistributionGGX(norm, halfDir, roughness);
		float G = GeometrySmith(norm, viewDir, lightDir, roughness);
		vec3 F = fresnelSchlick(max(dot(halfDir, viewDir), 0.0f), F0);
		vec3 numerator = NDF * G * F;
		float denominator = 4.0f * max(dot(norm, viewDir), 0.0f) * max(dot(norm, lightDir), 0.0f) + 0.0001f;
		vec3 specular = numerator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0f) - kS;
		kD *= 1.0f - metallic;

		float normDotLightDir = max(dot(norm, lightDir), 0.0f);
		l0 += (kD * albedo / pi + specular) * radiance * normDotLightDir;
	}

	vec3 kS = fresnelSchlickRoughness(max(dot(norm, viewDir), 0.0f), F0, roughness);
	vec3 kD = 1.0f - kS;
	vec3 irradiance = texture(irradianceSamp, norm).rgb;
	vec3 diffuse = irradiance * albedo;
	vec3 ambient = (kD * diffuse) * ao;

	//ambient = irradiance;

	fragColor = vec4(l0 + ambient, 1.0f);
	//fragColor = vec4(ambient, 1.0f);
}