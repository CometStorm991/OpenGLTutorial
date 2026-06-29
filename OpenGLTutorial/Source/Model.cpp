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

	processNode(scene->mRootNode, scene, convertMatrixToGLM(scene->mRootNode->mTransformation));
}

void Model::processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform)
{
	glm::mat4 nodeTransform = parentTransform * convertMatrixToGLM(node->mTransformation);

	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene, nodeTransform));
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, nodeTransform);
	}
}

glm::mat4 Model::convertMatrixToGLM(const aiMatrix4x4& from) {
	glm::mat4 to;
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene, glm::mat4 nodeTransform)
{
	std::vector<MeshVertex> vertices{};
	std::vector<uint32_t> indices{};
	std::vector<MeshTexture> textures{};

	int uvIndex = 0;
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiReturn succeeded = material->Get(AI_MATKEY_UVWSRC(aiTextureType_DIFFUSE, 0), uvIndex);
		if (succeeded != AI_SUCCESS) {
			std::cout << "Mesh " << mesh->mName.C_Str() << " texture UV index loading failed" << std::endl;
		}
	}

	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		MeshVertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;
		vertex.position = glm::vec3{ nodeTransform * glm::vec4{ vertex.position, 1.0f } };

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;
		vertex.normal = glm::vec3{ glm::transpose(glm::inverse(nodeTransform)) * glm::vec4{ vertex.normal, 0.0f } };
		
		if (mesh->mTextureCoords[uvIndex])
		{
			vertex.texCoords.x = mesh->mTextureCoords[uvIndex][i].x;
			vertex.texCoords.y = mesh->mTextureCoords[uvIndex][i].y;
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
		std::vector<MeshTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, MeshTextureType::DIFFUSE, scene);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<MeshTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, MeshTextureType::SPECULAR, scene);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh{ vertices, indices, textures, renderer };
}

std::vector<MeshTexture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, MeshTextureType meshTextureType, const aiScene* scene)
{
	std::vector<MeshTexture> textures;
	for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string pathStr = str.C_Str();

		MeshTexture texture;

		if (pathStr[0] == '*')
		{
			uint32_t index = std::stoi(&pathStr[1]);
			aiTexture* embeddedTexture = scene->mTextures[index];

			int width, height, channelCount;
			unsigned char* data = nullptr;

			if (embeddedTexture->mHeight == 0) {
				data = stbi_load_from_memory(
					reinterpret_cast<unsigned char*>(embeddedTexture->pcData),
					embeddedTexture->mWidth,
					&width, &height, &channelCount, 0
				);

				if (data) {
					texture.id = loadTexture(meshTextureType, width, height, channelCount, data);
					texture.type = meshTextureType;
					texture.path = std::string{ str.C_Str() };
					
					stbi_image_free(data);
				}
			}
		}
		else
		{
			texture.id = textureFromFile(str.C_Str(), directory, meshTextureType);
			texture.type = meshTextureType;
			texture.path = std::string{ str.C_Str() };
		}
		
		textures.push_back(texture);
	}

	return textures;
}

uint32_t Model::textureFromFile(const std::string& path, const std::string& directory, MeshTextureType meshTextureType)
{
	int width, height, channelCount;
	stbi_set_flip_vertically_on_load(false);
	std::string fullPath = directory + "/" + path;
	unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &channelCount, 0);
	if (!data)
	{
		std::cout << "[Error] Failed to load texture " << fullPath << std::endl;
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

	uint32_t id = loadTexture(meshTextureType, width, height, channelCount, data);
	stbi_image_free(data);
	return id;
}

uint32_t Model::loadTexture(MeshTextureType meshTextureType, int width, int height, int channelCount, unsigned char* data)
{
	uint32_t id;
	glCreateTextures(GL_TEXTURE_2D, 1, &id);

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

void Model::draw(uint32_t fbId, uint32_t programId, const glm::mat4& view, const glm::vec3& pos)
{
	for (Mesh mesh : meshes)
	{
		mesh.draw(fbId, programId, view, pos);
	}
}