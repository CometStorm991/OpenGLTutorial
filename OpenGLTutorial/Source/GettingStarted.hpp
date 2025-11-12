#include <random>
#include <vector>

#include "Cube.hpp"
#include "Camera.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"

class GettingStarted : public Demo
{
public:
	GettingStarted();

	void init() override;
	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;
private:
	Renderer renderer;
	Camera camera;

	uint32_t vaoId;
	std::vector<uint32_t> textureIds;
	uint32_t programId;

	const uint32_t cubeCount = 1000;
	std::vector<glm::vec3> cubePositions;
	std::vector<glm::vec3> cubeRotationSpeeds;

	void addCubeVertices(std::vector<uint32_t>& textureIds, uint32_t& vao);
};