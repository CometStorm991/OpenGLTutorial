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
	void processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene, glm::mat4 nodeTransform);
	std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, MeshTextureType meshTextureType, const aiScene* scene);
	uint32_t textureFromFile(const std::string& path, const std::string& directory, MeshTextureType meshTextureType);
	uint32_t loadTexture(MeshTextureType meshTextureType, int width, int height, int channelCount, unsigned char* data);

	glm::mat4 convertMatrixToGLM(const aiMatrix4x4& from);
};