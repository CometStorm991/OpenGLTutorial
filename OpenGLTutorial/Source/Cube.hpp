#pragma once

#include <vector>

class Cube
{
public:
	static void generatePSkybox(std::vector<float>& vertices);
	static void generatePT(std::vector<float>& vertices);
	static void generatePNT(std::vector<float>& vertices);
};