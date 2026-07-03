#include "UVSphere.hpp"

// Most code from https://danielsieger.com/blog/2021/03/27/generating-spheres.html
std::vector<float> UVSphere::fillP(uint32_t slices, uint32_t stacks)
{
	return flatten(generateMesh(slices, stacks));
}

std::vector<float> UVSphere::fillN(uint32_t slices, uint32_t stacks)
{
	std::vector<UVSphere::Triangle> mesh = generateMesh(slices, stacks);

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

		norms.push_back(Triangle{ norm, norm, norm });
	}

	return flatten(norms);
}

std::vector<float> UVSphere::fillT(uint32_t slices, uint32_t stacks)
{
	if (stacks < 2)
	{
		std::cerr << "[Error] Stack count " << stacks << " is less than 2" << std::endl;
	}

	std::vector<glm::vec2> points{};
	points.reserve(slices * (stacks - 1) + 2);

	// Top vertex
	points.push_back(glm::vec2{ 0.0f, 1.0f }); // Temporary
	const glm::vec2& v0 = points[points.size() - 1];

	float sliceWidth = 1.0f / slices;
	float stackHeight = 1.0f / stacks;

	std::vector<glm::vec2> topPoints{};
	topPoints.reserve(slices);
	for (uint32_t j = 0; j < slices; j++)
	{
		topPoints.push_back(glm::vec2{ std::fmodf(j + 0.5f, slices) * sliceWidth, 1.0f });
	}

	std::vector<glm::vec2> bottomPoints{};
	bottomPoints.reserve(slices);
	float sliceWidth = 1.0f / slices;
	for (uint32_t j = 0; j < slices; j++)
	{
		topPoints.push_back(glm::vec2{ std::fmodf(j + 0.5f, slices) * sliceWidth, 0.0f });
	}

	// Middle vertices
	for (uint32_t i = 0; i < stacks - 1; i++)
	{
		for (uint32_t j = 0; j < slices; j++)
		{
			points.push_back(glm::vec2{(i + 1) * stackHeight, j * sliceWidth});
		}
	}

	// Bottom vertex
	points.push_back(glm::vec2{ 0.0f, 0.0f }); // Temporary
	const glm::vec2& v1 = points[points.size() - 1];

	std::vector<UVTriangle> mesh{};

	// Adding top/bottom triangles
	for (uint32_t i = 0; i < slices; i++)
	{
		uint32_t i0 = i + 1;
		uint32_t i1 = (i + 1) % slices + 1;
		mesh.push_back(UVTriangle{ topPoints[i], points[i0], points[i1] });
		i0 = i + slices * (stacks - 2) + 1;
		i1 = (i + 1) % slices + slices * (stacks - 2) + 1;
		mesh.push_back(UVTriangle{ bottomPoints[i], points[i1], points[i0] });
	}

	// Add middle triangles
	for (uint32_t j = 0; j < stacks - 2; j++)
	{
		uint32_t j0 = j * slices + 1;
		uint32_t j1 = (j + 1) * slices + 1;
		for (uint32_t i = 0; i < slices; i++)
		{
			uint32_t i0 = j0 + i;
			uint32_t i1 = j0 + (i + 1) % slices;
			uint32_t i2 = j1 + (i + 1) % slices;
			uint32_t i3 = j1 + i;
			mesh.push_back(UVTriangle{ points[i0], points[i3], points[i1] });
			mesh.push_back(UVTriangle{ points[i2], points[i1], points[i3] });
		}
	}

	return flatten(mesh);
}

std::vector<float> UVSphere::fillTan(uint32_t slices, uint32_t stacks)
{
	if (stacks < 2)
	{
		std::cerr << "[Error] Stack count " << stacks << " is less than 2" << std::endl;
	}

	std::vector<glm::vec3> points{};
	points.reserve(slices * (stacks - 1) + 2);

	// Top vertex
	points.push_back(glm::vec3{ 0.0f, 1.0f, 0.0f }); // Temporary
	const glm::vec3& v0 = points[points.size() - 1];

	std::vector<glm::vec3> polePoints{};
	polePoints.reserve(slices);
	for (uint32_t j = 0; j < slices; j++)
	{
		float theta = 2.0f * std::numbers::pi * (static_cast<float>(j) + 0.5f) / static_cast<float>(slices);

		float tanX = -std::sinf(theta);
		float tanY = 0.0f;
		float tanZ = std::cosf(theta);

		polePoints.push_back(glm::vec3{ tanX, tanY, tanZ });
	}

	// Middle vertices
	for (uint32_t i = 0; i < stacks - 1; i++)
	{
		float phi = std::numbers::pi * static_cast<float>(i + 1) / static_cast<float>(stacks);
		for (uint32_t j = 0; j < slices; j++)
		{
			float theta = 2.0f * std::numbers::pi * static_cast<float>(j) / static_cast<float>(slices);

			float tanX = -std::sinf(theta);
			float tanY = 0.0f;
			float tanZ = std::cosf(theta);

			points.push_back(glm::vec3{ tanX, tanY, tanZ });
		}
	}

	// Bottom vertex
	points.push_back(glm::vec3{ 0.0f, -1.0f, 0.0f }); // Temporary
	const glm::vec3& v1 = points[points.size() - 1];

	std::vector<Triangle> mesh{};

	// Adding top/bottom triangles
	for (uint32_t i = 0; i < slices; i++)
	{
		uint32_t i0 = i + 1;
		uint32_t i1 = (i + 1) % slices + 1;
		mesh.push_back(Triangle{ polePoints[i], points[i0], points[i1]});
		i0 = i + slices * (stacks - 2) + 1;
		i1 = (i + 1) % slices + slices * (stacks - 2) + 1;
		mesh.push_back(Triangle{ polePoints[i], points[i1], points[i0] });
	}

	// Add middle triangles
	for (uint32_t j = 0; j < stacks - 2; j++)
	{
		uint32_t j0 = j * slices + 1;
		uint32_t j1 = (j + 1) * slices + 1;
		for (uint32_t i = 0; i < slices; i++)
		{
			uint32_t i0 = j0 + i;
			uint32_t i1 = j0 + (i + 1) % slices;
			uint32_t i2 = j1 + (i + 1) % slices;
			uint32_t i3 = j1 + i;
			mesh.push_back(Triangle{ points[i0], points[i3], points[i1] });
			mesh.push_back(Triangle{ points[i2], points[i1], points[i3] });
		}
	}

	return flatten(mesh);
}

std::vector<UVSphere::Triangle> UVSphere::generateMesh(uint32_t slices, uint32_t stacks)
{
	if (stacks < 2)
	{
		std::cerr << "[Error] Stack count " << stacks << " is less than 2" << std::endl;
	}

	std::vector<glm::vec3> points{};
	points.reserve(slices * (stacks - 1) + 2);

	// Top vertex
	points.push_back(glm::vec3{ 0.0f, 1.0f, 0.0f });
	const glm::vec3& v0 = points[points.size() - 1];

	// Middle vertices
	for (uint32_t i = 0; i < stacks - 1; i++)
	{
		float phi = std::numbers::pi * static_cast<float>(i + 1) / static_cast<float>(stacks);
		for (uint32_t j = 0; j < slices; j++)
		{
			float theta = 2.0f * std::numbers::pi * static_cast<float>(j) / static_cast<float>(slices);
			float x = std::sinf(phi) * std::cosf(theta);
			float y = std::cosf(phi);
			float z = std::sinf(phi) * std::sinf(theta);

			float tanX = -std::sinf(theta);
			float tanY = 0.0f;
			float tanZ = std::cosf(theta);

			points.push_back(glm::vec3{ x, y, z });
		}
	}

	// Bottom vertex
	points.push_back(glm::vec3{ 0.0f, -1.0f, 0.0f });
	const glm::vec3& v1 = points[points.size() - 1];

	std::vector<Triangle> mesh{};

	// Adding top/bottom triangles
	for (uint32_t i = 0; i < slices; i++)
	{
		uint32_t i0 = i + 1;
		uint32_t i1 = (i + 1) % slices + 1;
		mesh.push_back(Triangle{ v0, points[i0], points[i1] });
		i0 = i + slices * (stacks - 2) + 1;
		i1 = (i + 1) % slices + slices * (stacks - 2) + 1;
		mesh.push_back(Triangle{ v1, points[i1], points[i0] });
	}

	// Add middle triangles
	for (uint32_t j = 0; j < stacks - 2; j++)
	{
		uint32_t j0 = j * slices + 1;
		uint32_t j1 = (j + 1) * slices + 1;
		for (uint32_t i = 0; i < slices; i++)
		{
			uint32_t i0 = j0 + i;
			uint32_t i1 = j0 + (i + 1) % slices;
			uint32_t i2 = j1 + (i + 1) % slices;
			uint32_t i3 = j1 + i;
			mesh.push_back(Triangle{ points[i0], points[i3], points[i1] });
			mesh.push_back(Triangle{ points[i2], points[i1], points[i3] });
		}
	}

	return mesh;
}

std::vector<float> UVSphere::flatten(const std::vector<Triangle>& mesh)
{
	const float* ptr = glm::value_ptr(mesh[0].verts[0]);
	std::vector<float> flattened{ ptr, ptr + 3 * mesh[0].verts.size() * mesh.size() };
	return flattened;
}

std::vector<float> UVSphere::flatten(const std::vector<UVTriangle>& mesh)
{
	const float* ptr = glm::value_ptr(mesh[0].verts[0]);
	std::vector<float> flattened{ ptr, ptr + 2 * mesh[0].verts.size() * mesh.size() };
	return flattened;
}
