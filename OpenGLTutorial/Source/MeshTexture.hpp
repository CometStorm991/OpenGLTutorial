#pragma once

#include <cstdint>
#include <string>

enum class MeshTextureType
{
	DIFFUSE, SPECULAR
};

struct MeshTexture
{
	MeshTextureType type;
	uint32_t id;
	std::string path;
};