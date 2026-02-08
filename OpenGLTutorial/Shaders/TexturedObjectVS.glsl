layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in mat4 aModel;
layout(location = 7) in float aTexScale;
layout(location = 8) in vec3 aTang;
layout(location = 9) in vec3 aBi;

out vec3 norm;
out vec3 fragPos;
out vec2 texCoords;
out vec3 tang;
out vec3 bi;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * aModel * vec4(aPos, 1.0f);

	mat4 normMat = transpose(inverse(aModel));
	norm = vec3(normalize(normMat * vec4(aNorm, 1.0f)));
	tang = vec3(normalize(normMat * vec4(aTang, 1.0f)));
	bi = vec3(normalize(normMat * vec4(aBi, 1.0f)));
	

	fragPos = vec3(aModel * vec4(aPos, 1.0f));
	texCoords = aTexScale * aTexCoords;
}