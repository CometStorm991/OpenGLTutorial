#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <numbers>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class UVSphere
{
public:
	static std::vector<float> fillP(uint32_t slices, uint32_t stacks);
	static std::vector<float> fillN(uint32_t slices, uint32_t stacks);
	static std::vector<float> fillT(uint32_t slices, uint32_t stacks);
	static std::vector<float> fillTan(uint32_t slices, uint32_t stacks);
private:
	struct Triangle
	{
		std::array<glm::vec3, 3> verts;
	};
	
	struct UVTriangle
	{
		std::array<glm::vec2, 3> verts;
	};

	static std::vector<Triangle> generateMesh(uint32_t slices, uint32_t stacks);
	static std::vector<float> flatten(const std::vector<Triangle>& mesh);
	static std::vector<float> flatten(const std::vector<UVTriangle>& mesh);
};