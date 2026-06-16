#version 330 core

layout (location = 0) out vec4 color;

in vec2 textureCoords;
flat in uint texSampIndex;

uniform sampler2D screenTexture[4];

void main()
{
	color = texture(screenTexture[texSampIndex], textureCoords);
    switch (texSampIndex) {
        case 2u:
            color = vec4(texture(screenTexture[texSampIndex], textureCoords).rgb, 1.0f);
            break;
        case 3u:
            float spec = texture(screenTexture[texSampIndex], textureCoords).a;
            color = vec4(spec, spec, spec, 1.0f);
            break;
        default:
            color = texture(screenTexture[texSampIndex], textureCoords);
    }
    if (color == vec4(0.0f, 0.0f, 0.0f, 1.0f) || color == vec4(0.0f, 0.0f, 0.0f, 0.0f))
    {
        color = vec4(textureCoords, 1.0f, 1.0f);
    }
}