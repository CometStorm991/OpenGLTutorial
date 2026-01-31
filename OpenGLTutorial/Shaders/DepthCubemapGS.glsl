#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];
uniform int lightIndex;

out vec4 fragPos;

void main()
{
	for (int face = 0; face < 6; face++)
	{
		gl_Layer = 6 * lightIndex + face;
		for (int i = 0; i < 3; i++)
		{
			fragPos = gl_in[i].gl_Position;
			gl_Position = shadowMatrices[face] * fragPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}