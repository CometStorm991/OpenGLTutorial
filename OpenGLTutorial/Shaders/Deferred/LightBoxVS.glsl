#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in mat4 aModel;

flat out int vInstanceId;
out vec3 vNorm;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * aModel * vec4(aPos, 1.0f);
	vInstanceId = gl_InstanceID;
	vNorm = aNorm;
}