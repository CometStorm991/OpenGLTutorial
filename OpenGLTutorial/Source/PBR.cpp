#include "PBR.hpp"

void PBR::prepare()
{
	std::vector<float> vertices;
	uint32_t stride = 0;

	stride = renderer.addToData(vertices, UVSphere::fillP(slices, stacks), stride, 3);

	renderer.prepareForRun();
}

void PBR::run()
{
	renderer.prepareForFrame();

	renderer.bindFramebuffer(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Camera camera = camController.getCamera();
	glm::mat4 view = camera.getView();
	glm::vec3 pos = camController.getCamera().pos;

	renderer.unprepareForFrame();

	window.updateGLFW();
	camController.updateCamera(window.getInputState(), renderer.getFrameTimeMilliseconds());
}

bool PBR::shouldEnd()
{
	return window.getShouldClose();
}

void PBR::terminate()
{
	window.terminate();
}