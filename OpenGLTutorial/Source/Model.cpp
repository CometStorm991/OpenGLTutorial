#include "Model.hpp"

Model::Model(const std::string& path, Renderer& renderer)
	: renderer(renderer)
{
	loadModel(path);
}

void Model::loadModel(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "[Error]: Assimp - " << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<MeshVertex> vertices{};
	std::vector<uint32_t> indices{};
	std::vector<MeshTexture> textures{};

	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		MeshVertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;
		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;
		
		if (mesh->mTextureCoords[0])
		{
			vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertex.texCoords = glm::vec2{ 0.0f, 0.0f };
		}

		vertices.push_back(vertex);
	}

	for (uint32_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<MeshTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, MeshTextureType::DIFFUSE);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<MeshTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, MeshTextureType::SPECULAR);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh{ vertices, indices, textures, renderer };
}

std::vector<MeshTexture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, MeshTextureType meshTextureType)
{
	std::vector<MeshTexture> textures;
	for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		MeshTexture texture;
		texture.id = textureFromFile(str.C_Str(), directory, meshTextureType);
		texture.type = meshTextureType;
		texture.path = std::string{ str.C_Str() };
		textures.push_back(texture);
	}

	return textures;
}

uint32_t Model::textureFromFile(const std::string& path, const std::string& directory, MeshTextureType meshTextureType)
{
	uint32_t id;
	glCreateTextures(GL_TEXTURE_2D, 1, &id);

	int width, height, channelCount;
	stbi_set_flip_vertically_on_load(false);
	unsigned char* data = stbi_load((directory + "/" +  path).c_str(), &width, &height, &channelCount, 0);
	if (!data)
	{
		std::cout << "[Error] Failed to load texture " << (directory + path) << std::endl;
		std::cout << "Directory: " << directory << " Path: " << path << std::endl;
		std::cout << stbi_failure_reason() << std::endl;
	}

	if (width <= 0 || height <= 0) {
		std::cout << "[Error] Width or height is equal to or less than 0" << std::endl;
		std::cout << "Width is " << width << "Height is " << height << std::endl;
	}
	if (channelCount < 3 || channelCount > 4)
	{
		std::cout << "[Error]: Channel count is " << channelCount << std::endl;
	}

	GLenum internalFormat;
	GLenum pixelFormat;
	switch (channelCount)
	{
	case 3:
		internalFormat = meshTextureType == MeshTextureType::DIFFUSE ? GL_SRGB8 : GL_RGB8;
		pixelFormat = GL_RGB;
		break;
	case 4:
	default:
		internalFormat = meshTextureType == MeshTextureType::DIFFUSE ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		pixelFormat = GL_RGB8;
		break;
	}
	
	glTextureStorage2D(id, 1, internalFormat, width, height);
	glTextureSubImage2D(id, 0, 0, 0, width, height, pixelFormat, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);

	std::vector<TextureParameter> textureParameters = {
		{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
		{GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
		{GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR},
		{GL_TEXTURE_MAG_FILTER, GL_LINEAR}
	};
	for (TextureParameter textureParameter : textureParameters)
	{
		glTextureParameteri(id, textureParameter.getParameter(), textureParameter.getArgument());
	}

	uint32_t textureUnit = meshTextureType == MeshTextureType::DIFFUSE ? 0 : 1;  // RELIES on only 1 diffuse/specular samplers
	renderer.addTexture(id, GL_TEXTURE_2D, textureUnit);

	return id;
}

void Model::draw(uint32_t programId, const glm::mat4& view, const glm::vec3& pos)
{
	for (Mesh mesh : meshes)
	{
		mesh.draw(programId, view, pos);
	}
}