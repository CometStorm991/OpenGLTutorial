#version 460 core

in vec2 vTexCoords;

out float fragColor;

uniform sampler2D ssaoSamp;

uniform float noiseTexLen;

void main()
{
	vec2 texelSize = 1.0f / vec2(textureSize(ssaoSamp, 0));
	float result = 0.0f;
	int start = int(noiseTexLen / 2.0f);
	for (int x = -start; x < start; x++)
	{
		for (int y = -start; y < start; y++)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoSamp, vTexCoords + offset).r;
		}
	}
	fragColor = result / (noiseTexLen * noiseTexLen);
}