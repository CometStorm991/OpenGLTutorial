#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aModel;

out vec3 norm;
out vec3 fragPos;
out vec2 texCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * aModel * vec4(aPos, 1.0f);

	mat4 normMat = transpose(inverse(aModel));
	norm = vec3(normMat * vec4(aNorm, 1.0f));
	fragPos = vec3(aModel * vec4(aPos, 1.0f));
	texCoords = aTexCoords;
}