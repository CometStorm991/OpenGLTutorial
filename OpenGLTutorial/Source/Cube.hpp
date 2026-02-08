#pragma once

#include <vector>

// Most of the code in the Cube class comes from ChatGPT
class Cube
{
public:
	static std::vector<float> fillP();
	static std::vector<float> fillN();
	static std::vector<float> fillT();

	// Deprecated, kept for older demos
	static void generatePSkybox(std::vector<float>& vertices);
	static void generateP(std::vector<float>& vertices);
	static void generatePT(std::vector<float>& vertices);
	static void generatePN(std::vector<float>& vertices);
	static void generatePNT(std::vector<float>& vertices);
};