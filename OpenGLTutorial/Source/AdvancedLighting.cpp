#include "AdvancedLighting.hpp"

AdvancedLighting::AdvancedLighting()
{

}

void AdvancedLighting::prepare()
{
	// Floors
	glm::mat4 floorModel = glm::mat4(1.0f);
	floorModel = glm::scale(floorModel, glm::vec3{ 50.0f, 1.0f, 50.0f });
	floors.emplace_back(10.0f, floorModel);
	floorCount++;
	buildFloorCube();

	prepareFloor(floors);

	// Boxes
	prepareBoxes();

	// Lights
	for (uint32_t i = 0; i < 8; i++)
	{
		lights.emplace_back(
			glm::vec3{ 1.0f, 1.0f, 1.0f },
			glm::mat4{ 1.0f },
			glm::vec3{ 0.0f, 5.0f, 0.0f },
			glm::vec3{ 0.2f, 0.2f, 0.2f },
			glm::vec3{ 0.5f, 0.5f, 0.5f },
			glm::vec3{ 1.0f, 1.0f, 1.0f },
			1.0f, 0.07f, 0.017f);
		lightCount += 1;
	}

	prepareLight();

	// Light shadows (should be called only after prepareLight())
	prepareCubemapShadows();


	renderer.setUniform1i(floorVaoId, "depthCubemaps", depthCubemapTexUnit);
	renderer.setUniform1i(boxesVaoId, "depthCubemaps", depthCubemapTexUnit);
	floorTextureIds.push_back(depthCubemapTexId);
	boxesTextureIds.push_back(depthCubemapTexId);
	renderer.setUniform1f(floorVaoId, "farPlane", depthCubemapFar);
	renderer.setUniform1f(boxesVaoId, "farPlane", depthCubemapFar);

	camController.setCameraPos(glm::vec3(-5.0f, 2.0f, 0.0f));
	camController.setCameraOrientation(0.0f, 0.0f);

	renderer.prepareForRun();

	// [Warning]: Will break with deferred shading
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);

	// Gamma correction
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void AdvancedLighting::buildFloorCube()
{
	float floorCubeWidth = floorCubeSize * 2.0f;
	float floorCubeTexScale = floorCubeSize * 2.0f * (10.0f / 50.0f);

	glm::mat4 top = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	top = glm::translate(top, glm::vec3(0.0f, floorCubeSize, 0.0f));
	top = glm::scale(top, glm::vec3(floorCubeWidth, floorCubeThickness, floorCubeWidth));

	glm::mat4 bottom = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	bottom = glm::translate(bottom, glm::vec3(0.0f, -floorCubeSize, 0.0f));
	bottom = glm::scale(bottom, glm::vec3(floorCubeWidth, floorCubeThickness, floorCubeWidth));

	glm::mat4 right = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	right = glm::translate(right, glm::vec3(floorCubeSize, 0.0f, 0.0f));
	right = glm::scale(right, glm::vec3(floorCubeThickness, floorCubeWidth, floorCubeWidth));

	glm::mat4 left = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	left = glm::translate(left, glm::vec3(-floorCubeSize, 0.0f, 0.0f));
	left = glm::scale(left, glm::vec3(floorCubeThickness, floorCubeWidth, floorCubeWidth));

	glm::mat4 front = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	front = glm::translate(front, glm::vec3(0.0f, 0.0f, floorCubeSize));
	front = glm::scale(front, glm::vec3(floorCubeWidth, floorCubeWidth, floorCubeThickness));

	glm::mat4 back = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	back = glm::translate(back, glm::vec3(0.0f, 0.0f, -floorCubeSize));
	back = glm::scale(back, glm::vec3(floorCubeWidth, floorCubeWidth, floorCubeThickness));

	floors.emplace_back(floorCubeTexScale, top);
	floors.emplace_back(floorCubeTexScale, bottom);
	floors.emplace_back(floorCubeTexScale, right);
	floors.emplace_back(floorCubeTexScale, left);
	floors.emplace_back(floorCubeTexScale, front);
	floors.emplace_back(floorCubeTexScale, back);
	floorCount += 6;
}

void AdvancedLighting::prepareFloor(const std::vector<Floor>& floors)
{
	std::vector<float> vertices;
	Cube::generatePNT(vertices);
	uint32_t stride = 8;

	std::vector<float> tangentData;
	std::vector<float> bitangentData;
	for (int i = 0; i < 12; i++)
	{
		std::vector<glm::vec3> pos{};
		std::vector<glm::vec3> norm{};
		std::vector<glm::vec2> texCoords{};

		for (int j = 0; j < 3; j++)
		{
			pos.push_back({
				vertices[i * 3 * stride + j * stride + 0],
				vertices[i * 3 * stride + j * stride + 1],
				vertices[i * 3 * stride + j * stride + 2], });
			norm.push_back({
				vertices[i * 3 * stride + j * stride + 3],
				vertices[i * 3 * stride + j * stride + 4],
				vertices[i * 3 * stride + j * stride + 5], });
			texCoords.push_back({
				vertices[i * 3 * stride + j * stride + 6],
				vertices[i * 3 * stride + j * stride + 7], });
		}

		glm::vec3 edge1 = pos[1] - pos[0];
		glm::vec3 edge2 = pos[2] - pos[0];
		glm::vec2 deltaUV1 = texCoords[1] - texCoords[0];
		glm::vec2 deltaUV2 = texCoords[2] - texCoords[0];

		glm::vec3 tangent{};
		glm::vec3 bitangent{};

		// Calculating inverse of TBN
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		const float* tangentPtr = glm::value_ptr(tangent);
		tangentData.insert(tangentData.end(), tangentPtr, tangentPtr + 3);
		tangentData.insert(tangentData.end(), tangentPtr, tangentPtr + 3);
		tangentData.insert(tangentData.end(), tangentPtr, tangentPtr + 3);
		const float* bitangentPtr = glm::value_ptr(bitangent);
		bitangentData.insert(bitangentData.end(), bitangentPtr, bitangentPtr + 3);
		bitangentData.insert(bitangentData.end(), bitangentPtr, bitangentPtr + 3);
		bitangentData.insert(bitangentData.end(), bitangentPtr, bitangentPtr + 3);

		std::cout << tangent.x << ", " << tangent.y << ", " << tangent.z << std::endl;
	}
	stride = addToData(vertices, tangentData, stride, 3);
	stride = addToData(vertices, bitangentData, stride, 3);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib{ 3, GL_FLOAT, 0 };
	AttributeLayout normAttrib{ 3, GL_FLOAT, 1 };
	AttributeLayout texAttrib{ 2, GL_FLOAT, 2 };
	AttributeLayout tanAttrib{ 3, GL_FLOAT, 8 };
	AttributeLayout biAttrib{ 3, GL_FLOAT, 9 };

	std::vector<AttributeLayout> attribs = {
		posAttrib, normAttrib, texAttrib,
		tanAttrib, biAttrib
	};

	renderer.createVertexArray(floorVaoId, vertexBuffer, 0, attribs);

	// Array of structures to structure of arrays
	std::vector<float> modelData;
	std::vector<float> texScaleData;
	for (const Floor& floor : floors)
	{
		const float* modelPtr = glm::value_ptr(floor.modelMat);
		modelData.insert(modelData.end(), modelPtr, modelPtr + 16);
		texScaleData.push_back(floor.texScale);
	}
	std::vector<float> instData;
	uint32_t instStride = 0;
	instStride = addToData(instData, modelData, instStride, 16);
	instStride = addToData(instData, texScaleData, instStride, 1);
	
	uint32_t instBuffer;
	renderer.generateVertexBuffer(instBuffer, instData);

	std::vector<AttributeLayout> instAttribs = {
		{4, GL_FLOAT, 3},
		{4, GL_FLOAT, 4},
		{4, GL_FLOAT, 5},
		{4, GL_FLOAT, 6},
		{1, GL_FLOAT, 7}
	};

	renderer.addInstToVertexArray(floorVaoId, instBuffer, instAttribs);

	uint32_t texture0;
	renderer.generateResourceTexture2D(texture0, "Resources/TutorialBrickWallDiffuse.jpg", true, GL_SRGB8, GL_TEXTURE_2D, materialDiffuseTexUnit);
	uint32_t texture1;
	renderer.generateResourceTexture2D(texture1, "Resources/TutorialBrickWallNormal.jpg", true, GL_SRGB8, GL_TEXTURE_2D, materialNormalTexUnit);
	floorTextureIds.clear();
	floorTextureIds.push_back(texture0);
	floorTextureIds.push_back(texture1);

	renderer.generateProgram(floorProgramId, {
		{GL_VERTEX_SHADER, "Shaders/TexturedObjectVS.glsl", "#version 330 core"},
		{GL_FRAGMENT_SHADER, "Shaders/TexturedObjectFS.glsl", "#version 460 core"}, });

	renderer.setUniform1i(floorProgramId, "material.diffuse", materialDiffuseTexUnit);
	renderer.setUniform1i(floorProgramId, "material.normal", materialNormalTexUnit);
	renderer.setUniform1f(floorProgramId, "material.specular", 1.0f);
	renderer.setUniform1f(floorProgramId, "material.shininess", 4.0f);
	//applyLightUniforms(floorProgramId);
}

void AdvancedLighting::prepareBoxes()
{
	std::vector<float> vertices;
	Cube::generatePNT(vertices);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	AttributeLayout normAttrib = AttributeLayout(3, GL_FLOAT);
	AttributeLayout texAttrib = AttributeLayout(2, GL_FLOAT);

	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);
	attribs.push_back(normAttrib);
	attribs.push_back(texAttrib);

	renderer.generateVertexArray(boxesVaoId, vertexBuffer, 0, attribs);

	glm::mat4 model1 = glm::translate(glm::mat4{ 1.0f }, glm::vec3(0.0f, 2.0f, 5.0f));
	glm::mat4 model2 = glm::mat4{ 1.0f };
	model2 = glm::translate(model2, glm::vec3(0.0f, 2.0f, -5.0f));
	model2 = glm::rotate(model2, 3.14f / 4.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 model3 = glm::translate(glm::mat4{ 1.0f }, glm::vec3(0.0f, 1.0f, -8.0f));
	std::vector<glm::mat4> models{ model1, model2, model3 };
	glCreateBuffers(1, &vertexBuffer);
	glNamedBufferStorage(vertexBuffer, models.size() * sizeof(glm::mat4), models.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayVertexBuffer(boxesVaoId, 1, vertexBuffer, 0, sizeof(glm::mat4));

	uint32_t startAttrib = 3;
	for (uint32_t i = 0; i < 4; i++)
	{
		uint32_t attrib = startAttrib + i;

		glEnableVertexArrayAttrib(boxesVaoId, attrib);
		glVertexArrayAttribFormat(boxesVaoId, attrib, 4, GL_FLOAT, GL_FALSE, i * sizeof(glm::vec4));
		glVertexArrayAttribBinding(boxesVaoId, attrib, 1);
	}

	glVertexArrayBindingDivisor(boxesVaoId, 1, 1);

	uint32_t texture0;
	renderer.generateResourceTexture2D(texture0, "Resources/TutorialDiffuseMap.png", true, GL_TEXTURE_2D, materialDiffuseTexUnit);
	uint32_t texture1;
	renderer.generateResourceTexture2D(texture1, "Resources/TutorialSpecularMap.png", true, GL_TEXTURE_2D, materialSpecularTexUnit);
	boxesTextureIds.clear();
	boxesTextureIds.push_back(texture0);
	boxesTextureIds.push_back(texture1);

	renderer.generateProgram(boxesProgramId, "Shaders/TexDiffSpecVS.glsl", "Shaders/TexDiffSpecFS.glsl");

	renderer.setUniform1i(boxesProgramId, "material.diffuse", materialDiffuseTexUnit);
	renderer.setUniform1i(boxesProgramId, "material.specular", materialSpecularTexUnit);
	renderer.setUniform1f(boxesProgramId, "material.shininess", 4.0f);

	//applyLightUniforms(boxesProgramId);
}

void AdvancedLighting::prepareWalls()
{
	std::vector<float> vertices;
	uint32_t stride = 0;

	stride = addToData(vertices, Cube::fillP(), stride, 3);
	stride = addToData(vertices, Cube::fillN(), stride, 3);
	stride = addToData(vertices, Cube::fillT(), stride, 2);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);
}

void AdvancedLighting::testAddToData()
{
	uint32_t stride = 0;

	std::vector<float> vertices{};

	std::vector<float> cubeVertices = {
		// +Z face (front)
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// -Z face (back)
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// -X face (left)
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,

		// +X face (right)
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,

		// +Y face (top)
		0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f,

		0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// -Y face (bottom)
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	
	addToData(vertices, cubeVertices, stride, 3);
	stride += 3;

	std::vector<float> cubeNormals = {
		// +X face
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,

		 // -X face
		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,

		 // +Y face
		  0.0f,  1.0f,  0.0f,
		  0.0f,  1.0f,  0.0f,
		  0.0f,  1.0f,  0.0f,
		  0.0f,  1.0f,  0.0f,
		  0.0f,  1.0f,  0.0f,
		  0.0f,  1.0f,  0.0f,

		  // -Y face
		   0.0f, -1.0f,  0.0f,
		   0.0f, -1.0f,  0.0f,
		   0.0f, -1.0f,  0.0f,
		   0.0f, -1.0f,  0.0f,
		   0.0f, -1.0f,  0.0f,
		   0.0f, -1.0f,  0.0f,

		   // +Z face
			0.0f,  0.0f,  1.0f,
			0.0f,  0.0f,  1.0f,
			0.0f,  0.0f,  1.0f,
			0.0f,  0.0f,  1.0f,
			0.0f,  0.0f,  1.0f,
			0.0f,  0.0f,  1.0f,

			// -Z face
			 0.0f,  0.0f, -1.0f,
			 0.0f,  0.0f, -1.0f,
			 0.0f,  0.0f, -1.0f,
			 0.0f,  0.0f, -1.0f,
			 0.0f,  0.0f, -1.0f,
			 0.0f,  0.0f, -1.0f
	};
	addToData(vertices, cubeNormals, stride, 3);
	stride += 3;

	for (int i = 0; i < 36; i++)
	{
		for (int j = 0; j < stride; j++)
		{
			std::cout << vertices[i * stride + j] << ", ";
		}
		std::cout << "\n";
	}
}

uint32_t AdvancedLighting::addToData(std::vector<float>& vertices, const std::vector<float>& data, uint32_t oldFloatStride, uint32_t componentCount)
{
	if (oldFloatStride == 0)
	{
		vertices = data;
		return componentCount;
	}

	float truncVertexCount = (float)(vertices.size() / oldFloatStride);
	float exactVertexCount = (float)(vertices.size()) / (float)(oldFloatStride);
	if (std::abs(exactVertexCount - truncVertexCount) > 0.001f)
	{
		std::cerr << "[Error]: Vertex float count and stride are incompatible.\n";
		std::cerr << "Exact vertex count " << exactVertexCount << " does not match trunc vertex count " << truncVertexCount << "\n";
	}
	uint32_t vertexCount = vertices.size() / oldFloatStride;
	uint32_t newFloatStride = oldFloatStride + componentCount;

	for (uint32_t i = 0; i < vertexCount; i++)
	{
		for (uint32_t j = 0; j < componentCount; j++)
		{
			vertices.insert(vertices.begin() + (i * newFloatStride + oldFloatStride + j), data[i * componentCount + j]);
		}
	}

	return newFloatStride;
}

void AdvancedLighting::prepareLight()
{
	std::vector<float> vertices;
	Cube::generateP(vertices);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);

	renderer.generateVertexArray(lightVaoId, vertexBuffer, 0, attribs);

	uint32_t instBuffer;
	uint32_t instSize = sizeof(glm::mat4) + sizeof(glm::vec3);
	uint32_t instBufferSize = lights.size() * instSize;
	glCreateBuffers(1, &instBuffer);
	glNamedBufferStorage(instBuffer, instBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
	depthCubemapInstBufferId = instBuffer;
	depthCubemapInstSize = instSize;
	// filling up the buffer happens at the end of method
	glVertexArrayVertexBuffer(lightVaoId, 1, instBuffer, 0, instSize);

	uint32_t startAttrib = 3;
	for (uint32_t i = 0; i < 4; i++)
	{
		uint32_t attrib = startAttrib + i;

		glEnableVertexArrayAttrib(lightVaoId, attrib);
		glVertexArrayAttribFormat(lightVaoId, attrib, 4, GL_FLOAT, GL_FALSE, i * sizeof(glm::vec4));
		glVertexArrayAttribBinding(lightVaoId, attrib, 1);
	}

	startAttrib = 7;
	glEnableVertexArrayAttrib(lightVaoId, startAttrib);
	glVertexArrayAttribFormat(lightVaoId, startAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat4));
	glVertexArrayAttribBinding(lightVaoId, startAttrib, 1);

	glVertexArrayBindingDivisor(lightVaoId, 1, 1);

	renderer.generateProgram(lightProgramId, "Shaders/ALLightVS.glsl", "Shaders/ALLightFS.glsl");

	std::vector<GPULight> gpuLights;

	lightSSBSize = lights.size() * lightSize;
	glCreateBuffers(1, &lightSSBId);
	for (uint32_t i = 0; i < lights.size(); i++)
	{
		gpuLights.push_back({
			glm::vec4{ lights[i].pos, 0.0f },
			glm::vec4{ lights[i].ambient, 0.0f },
			glm::vec4{ lights[i].diffuse, 0.0f },
			glm::vec4{ lights[i].specular, 0.0f },
			glm::vec4{ lights[i].constant, lights[i].linear, lights[i].quadratic, 0.0f },
			});
	}
	glNamedBufferStorage(lightSSBId, lightSSBSize, gpuLights.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBId);

	updateLightData();
}

void AdvancedLighting::updateLightData()
{
	std::vector<GPULight> gpuLights;
	lightSSBSize = lights.size() * lightSize;

	for (uint32_t i = 0; i < lights.size(); i++)
	{
		glNamedBufferSubData(depthCubemapInstBufferId, i * depthCubemapInstSize, sizeof(glm::mat4), glm::value_ptr(lights[i].modelMat));
		glNamedBufferSubData(depthCubemapInstBufferId, i * depthCubemapInstSize + sizeof(glm::mat4), sizeof(glm::vec3), &(lights[i].color));

		
		gpuLights.push_back({
			glm::vec4{ lights[i].pos, 0.0f },
			glm::vec4{ lights[i].ambient, 0.0f },
			glm::vec4{ lights[i].diffuse, 0.0f },
			glm::vec4{ lights[i].specular, 0.0f },
			glm::vec4{ lights[i].constant, lights[i].linear, lights[i].quadratic, 0.0f },
			});
	}
	glNamedBufferSubData(lightSSBId, 0, lightSSBSize, gpuLights.data());
}

void AdvancedLighting::prepareCubemapShadows()
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &depthCubemapTexId);
	glTextureStorage3D(depthCubemapTexId, 1, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT, 6 * depthCubemapMaxCount);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	renderer.addTexture(depthCubemapTexId, GL_TEXTURE_CUBE_MAP_ARRAY, depthCubemapTexUnit);

	glCreateFramebuffers(1, &depthCubemapFbId);
	// Will need to replace depth attachment per light
	glNamedFramebufferTexture(depthCubemapFbId, GL_DEPTH_ATTACHMENT, depthCubemapTexId, 0);
	glNamedFramebufferDrawBuffer(depthCubemapFbId, GL_NONE);
	glNamedFramebufferReadBuffer(depthCubemapFbId, GL_NONE);

	renderer.generateProgram(depthCubemapProgId, {
		{GL_VERTEX_SHADER, "Shaders/DepthCubemapVS.glsl"},
		{GL_GEOMETRY_SHADER, "Shaders/DepthCubemapGS.glsl"},
		{GL_FRAGMENT_SHADER, "Shaders/DepthCubemapFS.glsl"} });

	if (glCheckNamedFramebufferStatus(depthCubemapFbId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "[Error] Framebuffer is not complete!" << std::endl;
	}

	renderer.setUniform1f(depthCubemapProgId, "farPlane", depthCubemapFar);
}

void AdvancedLighting::run()
{
	renderer.prepareForFrame();

	// Clears framebuffers
	renderer.bindFramebuffer(0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.bindFramebuffer(depthCubemapFbId);
	glClear(GL_DEPTH_BUFFER_BIT);

	uint64_t milliseconds = renderer.getMillisecondsSinceRunPreparation();
	float seconds = milliseconds / 1000.0f;
	for (uint32_t i = 0; i < lights.size(); i++)
	{
		Light& light = lights[i];

		float circleRadius = i + 1.0f;
		float circleSpeed = (i + 1) * 0.5f;

		light.pos = glm::vec3{ circleRadius * std::sinf(seconds * circleSpeed), 5.0f, circleRadius * std::cosf(seconds * circleSpeed) };

		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, light.pos);
		lightModel = glm::translate(lightModel, glm::vec3{ -0.5f, -0.5f, -0.5f });
		lightModel = glm::scale(lightModel, glm::vec3{ 0.5f, 0.5f, 0.5f });
		light.modelMat = lightModel;
	}
	updateLightData();

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	renderShadowCubemap();
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	renderScene();

	renderer.unprepareForFrame();

	window.updateGLFW();
	camController.updateCamera(window.getInputState(), renderer.getFrameTimeMilliseconds());

	//std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void AdvancedLighting::renderShadowCubemap()
{
	for (uint32_t i = 0; i < lights.size(); i++)
	{
		const Light& light = lights[i];
		renderer.setUniform3f(depthCubemapProgId, "lightPos", light.pos);
		renderer.setUniform1i(depthCubemapProgId, "lightIndex", i);

		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), SHADOW_ASPECT, depthCubemapNear, depthCubemapFar);
		const glm::vec3& lightPos = light.pos;
		std::vector<glm::mat4> shadowTransforms{};

		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos,
				lightPos + glm::vec3(1.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, -1.0f, 0.0f)));

		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos,
				lightPos + glm::vec3(-1.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, -1.0f, 0.0f)));

		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos,
				lightPos + glm::vec3(0.0f, 1.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 1.0f)));

		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos,
				lightPos + glm::vec3(0.0f, -1.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, -1.0f)));

		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos,
				lightPos + glm::vec3(0.0f, 0.0f, 1.0f),
				glm::vec3(0.0f, -1.0f, 0.0f)));

		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos,
				lightPos + glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec3(0.0f, -1.0f, 0.0f)));

		renderer.setUniformMatrix4fvArr(depthCubemapProgId, "shadowMatrices", 6, (float*)(shadowTransforms.data()));

		{
			renderer.prepareForDraw(depthCubemapFbId, depthCubemapProgId, {}, floorVaoId);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_CULL_FACE);

			renderer.drawInstanced(36, floorCount);

			renderer.unprepareForDraw(depthCubemapProgId, {});
		}

		/*
		{
			renderer.prepareForDraw(depthCubemapFbId, depthCubemapProgId, {}, lightVaoId);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_CULL_FACE);

			renderer.drawInstanced(36, lightCount);

			renderer.unprepareForDraw(depthCubemapProgId, {});
		}
		*/

		{
			renderer.prepareForDraw(depthCubemapFbId, depthCubemapProgId, {}, boxesVaoId);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_CULL_FACE);

			renderer.drawInstanced(36, boxesCount);

			renderer.unprepareForDraw(depthCubemapProgId, {});
		}
	}
}

void AdvancedLighting::renderScene()
{
	glm::mat4 view = camController.getCamera().getView();
	glm::vec3 pos = camController.getCamera().pos;

	{
		renderer.prepareForDraw(floorProgramId, floorTextureIds, floorVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(floorProgramId, "", "view", "projection");
		renderer.setUniform3f(floorProgramId, "viewPos", pos);
		renderer.drawInstanced(36, floorCount);

		renderer.unprepareForDraw(floorProgramId, floorTextureIds);
	}

	{
		renderer.prepareForDraw(lightProgramId, {}, lightVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(lightProgramId, "", "view", "projection");
		renderer.setUniform3f(lightProgramId, "viewPos", pos);
		renderer.drawInstanced(36, lightCount);

		renderer.unprepareForDraw(lightProgramId, {});
	}

	{
		renderer.prepareForDraw(boxesProgramId, boxesTextureIds, boxesVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(boxesProgramId, "", "view", "projection");
		renderer.setUniform3f(boxesProgramId, "viewPos", pos);
		renderer.drawInstanced(36, boxesCount);

		renderer.unprepareForDraw(boxesProgramId, boxesTextureIds);
	}
}

bool AdvancedLighting::shouldEnd()
{
	return window.getShouldClose();
}
void AdvancedLighting::terminate()
{
	window.terminate();
}