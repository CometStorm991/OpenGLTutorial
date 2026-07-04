#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTang;
layout(location = 4) in mat4 aModel;

uniform mat4 view;
uniform mat4 projection;

out vec3 vFragPos;
out vec3 vNorm;
out vec3 vTang;
out vec3 vBi;
out vec2 vTexCoords;

void main()
{
	gl_Position = projection * view * aModel * vec4(aPos, 1.0f);
	vFragPos = vec3(aModel * vec4(aPos, 1.0f));

	mat4 normMat = transpose(inverse(aModel));

	vec3 norm = normalize(vec3(normMat * vec4(aNorm, 1.0f)));
	vec3 tang = normalize(vec3(normMat * vec4(aTang, 1.0f)));
	vec3 bi = normalize(cross(norm, tang));

	vNorm = norm;
	vTang = tang;
	vBi = bi;

	vTexCoords = aTexCoords;
}