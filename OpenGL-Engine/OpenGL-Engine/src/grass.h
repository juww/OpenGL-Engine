#pragma once
#ifndef GRASS_H
#define GRASS_H

#include <vector>
#include <cmath>
#include <glm/glm.hpp>

#include "shader_m.h"

class Grass {
public:
	glm::vec3 pos, rot, scale;
	glm::mat4 model;
	unsigned int vao = 0, ebo = 0;
	unsigned int noiseTex = 0;
	int width, height;
	int density;
	int count;
	std::vector<float> noiseMap;

	void initialize(const int &w, const int &h, const int &d) {
		model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(2.0f));
		model = glm::translate(model, glm::vec3(-0.050f, 0.0f, 0.0f));

		width = w;
		height = h;
		density = d;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		std::vector<unsigned int> indices;
		for (int i = 0; i < N_VERTEX; i++) {
			indices.push_back(i);
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices.at(0), GL_STATIC_DRAW);

		unsigned int vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		unsigned int sizeBuffer = N_VERTEX * SIZEPOSITION * SIZETEXCOORD * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, sizeBuffer, &vectices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) (sizeof(float) * 3));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}

	void generateNoiseMap(Shader *shader, int seed, float scale, int octaves, float persistence, float lacunarity, glm::vec2 offset) {

		if (noiseTex != 0) {
			glDeleteTextures(1, &noiseTex);
			noiseTex = 0;
		}
		Noise noise;
		std::vector<std::vector<float> > perlinNoiseMap = noise.GenerateNoiseMap(width, height, seed, scale, octaves, persistence, lacunarity, offset, noise.Local);
		noiseMap.resize(width * height);
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				noiseMap[j + (height * i)] = perlinNoiseMap[i][j];
			}
		}
		glGenTextures(1, &noiseTex);
		glBindTexture(GL_TEXTURE_2D, noiseTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// float = 4 byte
		// to do: reduce size using unsigned byte = 1 byte
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, &noiseMap.at(0));
		glGenerateMipmap(GL_TEXTURE_2D);

		shader->use();
		shader->setInt("noiseMap", 0);
	}

	void setPositionGrass(const std::vector<glm::vec3>& posOffset, const std::vector<float> &rad) {

		if (vao == 0) return;
		glBindVertexArray(vao);

		unsigned int instanceVbo;
		glGenBuffers(1, &instanceVbo);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);

		glBufferData(GL_ARRAY_BUFFER, posOffset.size() * sizeof(float) * 3, &posOffset.at(0), GL_STATIC_DRAW);
		count = posOffset.size();
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(6);
		
		unsigned int radVbo;
		glGenBuffers(1, &radVbo);
		glBindBuffer(GL_ARRAY_BUFFER, radVbo);

		glBufferData(GL_ARRAY_BUFFER, rad.size() * sizeof(float), &rad.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
		glEnableVertexAttribArray(7);

		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);

		printf("count Grass = %d\n", count);
		glBindVertexArray(0);
	}

	void update() {

	}

	void draw(Shader* shader, const glm::mat4& projection, const glm::mat4& view, const float &_time, const float &F, const float &A, const float &scl, const float &drp) {

		shader->use();
		
		shader->setMat4("model", model);
		shader->setMat4("projection", projection);
		shader->setMat4("view", view);
		shader->setFloat("halfOffset", float(width) / 2.0f + 0.5f);
		shader->setFloat("deltaTime", _time);
		shader->setFloat("length", width);
		shader->setFloat("frequency", F);
		shader->setFloat("amplitude", A);
		shader->setFloat("_scale", scl);
		shader->setFloat("_droop", drp);

		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, noiseTex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElementsInstanced(GL_TRIANGLE_STRIP, N_VERTEX, GL_UNSIGNED_INT, (void*)0, count);

		glBindVertexArray(0);
	}
private:
    static const int N_VERTEX = 15;
    const float vectices[N_VERTEX * (3 + 2)] = {
        // position				texcoord
        0.012f, 0.000f, 0.000f, 0.400f, 1.000f,
        0.088f, 0.000f, 0.000f, 0.500f, 1.000f,
        0.013f, 0.100f, 0.000f, 0.400f, 0.900f,
        0.087f, 0.100f, 0.000f, 0.500f, 0.900f,
        0.016f, 0.187f, 0.000f, 0.400f, 0.800f,
        0.084f, 0.187f, 0.000f, 0.500f, 0.800f,
        0.021f, 0.264f, 0.000f, 0.400f, 0.700f,
        0.079f, 0.264f, 0.000f, 0.500f, 0.700f,
        0.025f, 0.336f, 0.000f, 0.400f, 0.600f,
        0.075f, 0.336f, 0.000f, 0.500f, 0.600f,
        0.031f, 0.394f, 0.000f, 0.400f, 0.500f,
        0.068f, 0.394f, 0.000f, 0.500f, 0.500f,
        0.038f, 0.440f, 0.000f, 0.400f, 0.400f,
        0.061f, 0.440f, 0.000f, 0.500f, 0.400f,
        0.050f, 0.485f, 0.000f, 0.450f, 0.300f,
    };
};

#endif