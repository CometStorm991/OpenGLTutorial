#include <random>
#include <vector>

#include "CameraController.hpp"
#include "Cube.hpp"
#include "Camera.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

class GettingStarted : public Demo
{
public:
	GettingStarted();

	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;
private:
	Window window{};
	Renderer renderer{};
	CameraController camController{};

	uint32_t vaoId;
	std::vector<uint32_t> textureIds;
	uint32_t programId;

	const uint32_t cubeCount = 1000;
	std::vector<glm::vec3> cubePositions;
	std::vector<glm::vec3> cubeRotationSpeeds;

	void addCubeVertices(std::vector<uint32_t>& textureIds, uint32_t& vao);
};