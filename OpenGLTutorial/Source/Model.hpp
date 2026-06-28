#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Mesh.hpp"

class Model
{
public:
	Model(const std::string& path, Renderer& renderer);
	void draw(uint32_t fbId, uint32_t programId, const glm::mat4& view, const glm::vec3& pos);
private:
	Renderer& renderer;

	std::vector<Mesh> meshes;
	std::string directory;

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, MeshTextureType meshTextureType);
	uint32_t textureFromFile(const std::string& path, const std::string& directory, MeshTextureType meshTextureType);
};