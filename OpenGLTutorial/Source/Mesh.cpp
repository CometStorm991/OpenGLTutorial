#include "Mesh.hpp"

Mesh::Mesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices, std::vector<MeshTexture> textures, Renderer& renderer)
	: vertices(vertices), indices(indices), textures(textures), renderer(renderer)
{
	setupMesh();
}

void Mesh::setupMesh()
{
	const float* vertexPtr = reinterpret_cast<float*>(vertices.data());
	std::vector<float> floatVertices{ vertexPtr, vertexPtr + 8 * vertices.size() }; // Relies on Vertex being 3, 3, 2 floats
	renderer.generateVertexBuffer(vertexBufferId, floatVertices);

	const uint32_t* indexPtr = reinterpret_cast<uint32_t*>(indices.data());
	std::vector<uint32_t> uintIndices{ indexPtr, indexPtr + 8 * indices.size() };
	renderer.generateIndexBuffer(indexBufferId, uintIndices);

	AttributeLayout posAttrib{ 3, GL_FLOAT, 0 };
	AttributeLayout normAttrib{ 3, GL_FLOAT, 1 };
	AttributeLayout texAttrib{ 2, GL_FLOAT, 2 };

	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);
	attribs.push_back(normAttrib);
	attribs.push_back(texAttrib);

	renderer.generateVertexArray(vertexArrayId, vertexBufferId, indexBufferId, attribs);
}

void Mesh::draw(uint32_t programId, const glm::mat4& view, const glm::vec3 pos)
{
	renderer.setUniform1i(programId, "material.diffuseSamp", 0);
	renderer.setUniform1i(programId, "material.specularSamp", 1);

	std::vector<uint32_t> textureIds;
	textureIds.reserve(textures.size());
	for (MeshTexture texture : textures)
	{
		textureIds.push_back(texture.id);
	}

	{
		renderer.prepareForDraw(0, programId, textureIds, vertexArrayId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(programId, "", "view", "projection");
		renderer.setUniform3f(programId, "viewPos", pos);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		renderer.unprepareForDraw(programId, textureIds);
	}
}