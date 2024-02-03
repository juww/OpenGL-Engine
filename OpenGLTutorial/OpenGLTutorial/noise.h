#pragma once
#ifndef NOISE_H
#define NOISE_H

#include <glm/glm.hpp>
#include <vector>

class Noise {
public:

	std::vector<std::vector<float>> GenerateNoiseMap(int mapWidth, int mapHeight, float scale) {
		if (scale <= 0) {
			scale = 0.000001f;
		}
		std::vector<std::vector<float>> noiseMap;

		for (int y = 0; y < mapHeight; y++) {
			for (int x = 0; x < mapWidth; x++) {
				float sampleX = x / scale;
				float sampleY = y / scale;

				//noise;
			}
		}

		return noiseMap;
	}
};


#endif