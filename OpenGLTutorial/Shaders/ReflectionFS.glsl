#version 330 core

out vec4 fragColor;

in vec3 normal;
in vec3 position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

uniform int reflective;

void main()
{
	
	vec3 normalizedViewDir = normalize(position - cameraPos);
	vec3 normalizedNorm = normalize(normal);

	vec3 reflectDir;
	if (reflective == 1)
	{
		reflectDir = reflect(normalizedViewDir, normalizedNorm);
	} else {
		// refractive
		reflectDir = refract(normalizedViewDir, normalizedNorm, 0.67f);
	}
	

	fragColor = vec4(texture(skybox, reflectDir).rgb, 1.0f);
}