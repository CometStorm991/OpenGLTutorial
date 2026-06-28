#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "MeshVertex.hpp"
#include "MeshTexture.hpp"
#include "Program.hpp"
#include "Renderer.hpp"

class Mesh
{
public:
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<MeshTexture> textures;

	Mesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices, std::vector<MeshTexture> textures, Renderer& renderer);
	void draw(uint32_t fbId, uint32_t programId, const glm::mat4& view, const glm::vec3 pos);
private:
	Renderer& renderer;

	uint32_t vertexArrayId, vertexBufferId, indexBufferId;
	void setupMesh();
};