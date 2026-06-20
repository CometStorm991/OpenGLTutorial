#pragma once

#include <array>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Isocahedron
{
public:
	static std::vector<float> fillP(uint32_t subdivisions);
	static std::vector<float> fillN(uint32_t subdivisions);
private:
	struct Triangle
	{
		std::array<glm::vec3, 3> verts;

		//std::vector<float> flatten();
	};

	static std::vector<Triangle> generateMesh(uint32_t subdivisions);
	static std::vector<Triangle> generateInitIsoc();
	static void subdivide(std::vector<Triangle>& mesh);
	static void projectOntoSphere(std::vector<Triangle>& mesh);
	static std::vector<float> flatten(std::vector<Triangle>& mesh);
};