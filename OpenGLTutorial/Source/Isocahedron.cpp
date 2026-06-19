#include "Isocahedron.hpp"

std::vector<float> Isocahedron::fillP()
{
	std::vector<Triangle> mesh = generateInitIsoc();

	projectOntoSphere(mesh);

	return flatten(mesh);
}

std::vector<float> Isocahedron::fillN()
{
	std::vector<Triangle> mesh = generateInitIsoc();
	projectOntoSphere(mesh);

	std::vector<Triangle> norms{};
	norms.reserve(mesh.size());
	for (uint32_t i = 0; i < mesh.size(); i++)
	{
		Triangle& tri = mesh[i];
		
		glm::vec3 sum{ 0.0f, 0.0f, 0.0f };
		for (uint32_t j = 0; j < tri.verts.size(); j++)
		{
			sum += tri.verts[j];
		}

		glm::vec3 avg = sum / static_cast<float>(tri.verts.size());
		glm::vec3 norm = glm::normalize(avg);

		norms.push_back(Triangle{norm, norm, norm});
	}

	return flatten(norms);
}

std::vector<Isocahedron::Triangle> Isocahedron::generateInitIsoc()
{
	float phi = (1.0f + sqrt(5.0f)) * 0.5f; // golden ratio
	float a = 1.0f;
	float b = 1.0f / phi;

	std::array<glm::vec3, 12> points =
	{
		glm::vec3{0.0f, b, -a},
		glm::vec3{b, a, 0.0f},
		glm::vec3{-b, a, 0.0f},
		glm::vec3{0.0f, b, a},
		glm::vec3{0.0f, -b, a},
		glm::vec3{-a, 0.0f, b},
		glm::vec3{0.0f, -b, -a},
		glm::vec3{a, 0.0f, -b},
		glm::vec3{a, 0.0f, b},
		glm::vec3{-a, 0.0f, -b},
		glm::vec3{b, -a, 0.0f},
		glm::vec3{-b, -a, 0.0f}
	};

	std::vector<Triangle> mesh =
	{
		{points[0], points[2], points[1]},
		{points[0], points[1], points[7]},
		{points[0], points[9], points[2]},
		{points[0], points[7], points[6]},
		{points[0], points[6], points[9]},
		{points[1], points[2], points[3]},
		{points[1], points[3], points[8]},
		{points[1], points[8], points[7]},
		{points[2], points[5], points[3]},
		{points[2], points[9], points[5]},
		{points[3], points[5], points[4]},
		{points[3], points[4], points[8]},
		{points[4], points[5], points[11]},
		{points[4], points[10], points[8]},
		{points[4], points[11], points[10]},
		{points[5], points[9], points[11]},
		{points[6], points[7], points[10]},
		{points[6], points[11], points[9]},
		{points[6], points[10], points[11]},
		{points[7], points[8], points[10]},
	};

	return mesh;
}

void Isocahedron::projectOntoSphere(std::vector<Triangle>& mesh)
{
	for (uint32_t i = 0; i < mesh.size(); i++)
	{
		Triangle& tri = mesh[i];
		for (uint32_t j = 0; j < tri.verts.size(); j++)
		{
			tri.verts[j] /= glm::length(tri.verts[j]);
		}
	}
}

std::vector<float> Isocahedron::flatten(std::vector<Triangle>& mesh)
{
	const float* ptr = glm::value_ptr(mesh[0].verts[0]);
	std::vector<float> flattened{ ptr, ptr + 3 * mesh[0].verts.size() * mesh.size() };
	return flattened;
}

//std::vector<float> Isocahedron::Triangle::flatten()
//{
//	const float* ptr = glm::value_ptr(verts[0]);
//	std::vector<float> flattened{ ptr, ptr + 3 * verts.size() };
//	return flattened;
//}