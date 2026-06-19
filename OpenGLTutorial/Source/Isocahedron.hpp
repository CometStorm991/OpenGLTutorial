#pragma once

#include <array>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Isocahedron
{
public:
	static std::vector<float> fillP();
	static std::vector<float> fillN();
private:
	struct Triangle
	{
		std::array<glm::vec3, 3> verts;

		//std::vector<float> flatten();
	};

	static std::vector<Triangle> generateInitIsoc();
	static void projectOntoSphere(std::vector<Triangle>& mesh);
	static std::vector<float> flatten(std::vector<Triangle>& mesh);
};