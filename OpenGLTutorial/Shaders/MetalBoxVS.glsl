#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

uniform float time;
uniform mat4 centerModel;
uniform mat4 ringModel;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 norm;

const float PI = 3.14159265358979323846;

mat4 rotationMat(vec3 axis, float angle)
{
    float sinVal = sin(angle);
    float cosVal = cos(angle);

    axis = normalize(axis);

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    float oneMinusCos = 1.0 - cosVal;

    return mat4(
        cosVal + x*x*oneMinusCos,        y*x*oneMinusCos + z*sinVal,  z*x*oneMinusCos - y*sinVal,  0.0f,
        x*y*oneMinusCos - z*sinVal,      cosVal + y*y*oneMinusCos,    z*y*oneMinusCos + x*sinVal,  0.0f,
        x*z*oneMinusCos + y*sinVal,      y*z*oneMinusCos - x*sinVal,  cosVal + z*z*oneMinusCos,    0.0f,
        0.0f,                            0.0f,                        0.0f,                        1.0f
    );
}
mat4 translationMat(vec3 t)
{
    return mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        t.x,  t.y,  t.z,  1.0f
    );
}
mat4 scaleMat(vec3 s)
{
    return mat4(
        s.x,  0.0f, 0.0f, 0.0f,
        0.0f, s.y,  0.0f, 0.0f,
        0.0f, 0.0f, s.z,  0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

uint xorshift32(inout uint seed) {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}
// range is 0 inclusive to 1 exclusive
float rand01(inout uint seed) {
    return float(xorshift32(seed) >> 8) * (1.0 / 16777216.0);
}
float randRange(inout uint seed, float min, float max)
{
    return min + rand01(seed) * (max - min);
}
vec3 randVec3(inout uint seed, float min, float max) {
    return vec3(randRange(seed, min, max), randRange(seed, min, max), randRange(seed, min, max));
}

void main()
{
    uint seed = uint(gl_InstanceID) ^ 0xA511E9B3u;
    rand01(seed); rand01(seed); rand01(seed);

	mat4 model;
	if (gl_InstanceID == 0)
	{
		model = centerModel;
        model = translationMat(vec3(-0.5f, -0.5f, -0.5f)) * model;
        model = scaleMat(vec3(10.0f, 10.0f, 10.0f)) * model;
        model = rotationMat(randVec3(seed, -1.0f, 1.0f), time) * model;
        // model = translationMat(vec3(0.5f, 0.5f, 0.5f)) * model;
	}
    else
    {
        model = ringModel;

        float rotationSpeed = randRange(seed, 0.0f, 5.0f);
        
        float revStartAngle = randRange(seed, 0.0f, 2.0f * PI);
        float revSpeed = randRange(seed, 0.5f, 1.5f);
        float revDist = randRange(seed, 15.0f, 20.0f);
        float revAngle = revStartAngle + time * revSpeed;

        float size = randRange(seed, 0.5f, 1.5f);

        model = translationMat(vec3(-0.5f, -0.5f, -0.5f)) * model;
        model = scaleMat(vec3(size, size, size)) * model;
        model = rotationMat(vec3(1.0f, 1.0f, 1.0f), time * rotationSpeed) * model;
        model = translationMat(vec3(revDist, 0.0f, 0.0f)) * model;
        model = rotationMat(vec3(0.0f, 1.0f, 0.0f), revAngle) * model;
    }

	fragPos = vec3(model * vec4(aPos, 1.0f));
	norm = vec3(transpose(inverse(model)) * vec4(aNorm, 1.0f));

	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}