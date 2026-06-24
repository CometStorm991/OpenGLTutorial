#pragma once

#include <cstdint>

enum class MeshTextureType
{
	DIFFUSE, SPECULAR
};

struct MeshTexture
{
	MeshTextureType type;
	uint32_t id;
};