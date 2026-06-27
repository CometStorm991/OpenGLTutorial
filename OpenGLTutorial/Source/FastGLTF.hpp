#pragma once

#include <iostream>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <STB/stb_image.h>

#include "Renderer.hpp"

// Most code in here is from https://github.com/spnda/fastgltf/blob/main/examples/gl_viewer/gl_viewer.cpp

bool loadAsset(const std::filesystem::path& path, fastgltf::Asset& resultAsset)
{
	fastgltf::Parser parser;

	fastgltf::Expected<fastgltf::MappedGltfFile> gltfFile = fastgltf::MappedGltfFile::FromPath(path);
	if (!bool(gltfFile))
	{
		std::cerr << "[Error] FastGLTF - Failed to open glTF file: " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
		return false;
	}

	fastgltf::Options gltfOptions =
		fastgltf::Options::GenerateMeshIndices |
		fastgltf::Options::LoadExternalBuffers;

	fastgltf::Expected<fastgltf::Asset> asset = parser.loadGltf(gltfFile.get(), path.parent_path(), gltfOptions);
	if (asset.error() != fastgltf::Error::None)
	{
		std::cerr << "Failed to load glTF: " << fastgltf::getErrorMessage(asset.error()) << '\n';
		return false;
	}

	resultAsset = std::move(asset.get());
}

void loadImage(fastgltf::Image& image, const fastgltf::Asset& asset)
{
    // Not doing mipmaps for simplicity's sake

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    std::visit(fastgltf::visitor{
        [](auto& arg) {},
        [&](fastgltf::sources::URI& filePath) {
            assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
            assert(filePath.uri.isLocalPath()); // We're only capable of loading local files.
            int width, height, channelCount;

            const std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &channelCount, 0);
            if (!data)
            {
                std::cout << "[Error] Failed to load texture " << path << std::endl;
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

            glTextureStorage2D(texture, 0, GL_RGBA8, width, height);
            glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        },
        [&](fastgltf::sources::Array& vector) {
            int width, height, nrChannels;
            unsigned char* data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data()), static_cast<int>(vector.bytes.size()), &width, &height, &nrChannels, 4);
            glTextureStorage2D(texture, 0, GL_RGBA8, width, height);
            glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        },
        [&](fastgltf::sources::BufferView& view) {
            auto& bufferView = asset.bufferViews[view.bufferViewIndex];
            auto& buffer = asset.buffers[bufferView.bufferIndex];
            // Yes, we've already loaded every buffer into some GL buffer. However, with GL it's simpler
            // to just copy the buffer data again for the texture. Besides, this is just an example.
            std::visit(fastgltf::visitor {
                // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning
                // all buffers are already loaded into a vector.
                [](auto& arg) {},
                [&](fastgltf::sources::Array& vector) {
                    int width, height, nrChannels;
                    unsigned char* data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data() + bufferView.byteOffset),
                                                                static_cast<int>(bufferView.byteLength), &width, &height, &nrChannels, 4);
                    glTextureStorage2D(texture, 0, GL_RGBA8, width, height);
                    glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    stbi_image_free(data);
                }
            }, buffer.data);
        },
    }, image.data);

    Renderer::addTexture();
}

void loadModel(const std::string& path)
{
	fastgltf::Asset asset;
	loadAsset(path, asset);
	for (fastgltf::Image& image : asset.images)
	{
		loadImage(image, asset);
	}
}