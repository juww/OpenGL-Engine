#pragma once
#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>

#include "shader_m.h"
#include "noise.h"

class Plane {

public:

	unsigned int ebo, vao;
	unsigned int noiseTex;

	unsigned int indicesCount;
	unsigned int componentType;

	// use x and z; y height
	Plane(const int& planeSize) {

		printf("plane %f\n", planeSize);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		generatePlane(planeSize);

		glBindVertexArray(0);
	}

	void GenerateNoiseMap(int width, int height, float scale, int octaves, float persistence, float lacunarity) {
		Noise noise;
		std::vector<std::vector<float>> noiseMap = noise.GenerateNoiseMap(width, height, scale, octaves, persistence, lacunarity);

		printf("generate Noise map finished\n");
		printf("noiseMap size y = %d\n", noiseMap.size());
		printf("noiseMap size x = %d\n", noiseMap[0].size());
		std::vector<glm::vec4> aColor;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				aColor.push_back(glm::vec4(noiseMap[y][x], noiseMap[y][x], noiseMap[y][x], 1.0f));
			}
		}
		printf("acolor size = %d\n", aColor.size());
		glBindVertexArray(vao);

		glGenTextures(1, &noiseTex);
		glBindTexture(GL_TEXTURE_2D, noiseTex);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, &aColor.at(0));

		glBindVertexArray(0);
	}

	void draw(Shader& shader, const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model, const int &np) {

		shader.use();
		shader.setInt("noiseMap", 0);

		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", model);
		shader.setFloat("lenght", np + 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, noiseTex);

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElements(GL_TRIANGLE_STRIP, indicesCount, componentType, (void*)(0));

		glBindVertexArray(0);
	}

	~Plane() {

	}

private:

	void generatePlane(const int &n) {
		
		printf("masuk ke plane indices\n");
		std::vector<unsigned int> indices;

		unsigned int indx = 0;
		for (int row = 0; row < n; row++) {
			// additional triangle on left
			indices.push_back(indx++);
			for (int col = 0; col < n + 1; col++) {
				// below
				indices.push_back(indx++);
				// upper
				indices.push_back(indx++);
			}
			// additional triangle on right
			indices.push_back(indx++);
		}

		glBindVertexArray(vao);

		indicesCount = indices.size() * sizeof(unsigned int);
		componentType = GL_UNSIGNED_INT;

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount, &indices.at(0), GL_STATIC_DRAW);

		unsigned int vboIds;
		glGenBuffers(1, &vboIds);
		glBindBuffer(GL_ARRAY_BUFFER, vboIds);
		glBufferData(GL_ARRAY_BUFFER, indicesCount, &indices.at(0), GL_STATIC_DRAW);

		glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned int), (void*)0);
		glEnableVertexAttribArray(0);

		printf("size indices: %d\n", indices.size());
	}
};

#endif
