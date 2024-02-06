#pragma once
#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>

#include "shader_m.h"

class Plane {

public:

	unsigned int ebo, vao, vbo;

	float width;
	float length;
	float division;
	
	std::vector<float> attributes;
	std::vector<unsigned int> indices;

	// use x and z; y height
	Plane(const float& w, const float& l, const float& div) {

		width = w;
		length = l;
		division = div;
		printf("plane %f %f %f\n", width, length, div);

		planeVertices(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(width, 0.0f, 0.0f), glm::vec3(width, 0.0f, length), glm::vec3(0.0f, 0.0f, length), division);
		genPlaneIndices(division);

		loadAttributePlane();

	}

	~Plane() {

	}

private:

	void loadAttributePlane() {

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices.at(0), GL_STATIC_DRAW);

		printf("sizeof attributes = %d\n", sizeof(attributes));

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, attributes.size() * sizeof(float) * 11, &attributes.at(0), GL_STATIC_DRAW);

		//position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		// tangent
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);
	}

	void planeVertices(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, float div) {

												// v3-----v2
		glm::vec3 vec03 = (v3 - v0) / div;		// |       |
		glm::vec3 vec12 = (v2 - v1) / div;		// |       |
												// v0-----v1
		printf("vec03 - (%f, %f, %f)\n", vec03.x, vec03.y, vec03.z);
		printf("vec12 - (%f, %f, %f)\n", vec12.x, vec12.y, vec12.z);

		const int n = div + 1;
		for (int row = 0; row < n; row++) {
			glm::vec3 start = v0 + vec03 * (float)row;
			glm::vec3 end = v1 + vec12 * (float)row;
			glm::vec3 vectorDiv = (end - start) / div;
			for (int col = 0; col < n; col++) {
				glm::vec3 line = start + vectorDiv * (float)col;
				// position
				attributes.push_back(line.x);
				attributes.push_back(line.y);
				attributes.push_back(line.z);

				// Tangent space Vectors;
				glm::vec3 tangent = glm::normalize(vectorDiv);
				glm::vec3 nextStart = start + vec03;
				glm::vec3 nextEnd = end + vec12;
				glm::vec3 nextVectorDiv = (nextEnd - nextStart) / div;
				glm::vec3 nextRowCrntVec = nextStart + nextVectorDiv * float(col);
				glm::vec3 possibleBitangent = glm::normalize(nextRowCrntVec - line);
				// normal
				glm::vec3 normal = glm::cross(tangent, possibleBitangent);
				attributes.push_back(normal.x);
				attributes.push_back(normal.y);
				attributes.push_back(normal.z);
				// texture / UV
				attributes.push_back(float(col) / div);
				attributes.push_back(float(row) / div);
				// tangent
				attributes.push_back(tangent.x);
				attributes.push_back(tangent.y);
				attributes.push_back(tangent.z);
				// Bitangent
				// The Bitangent could also be the opposite of this
				//attributes.push_back(possibleBitangent.x);
				//attributes.push_back(possibleBitangent.y);
				//attributes.push_back(possibleBitangent.z);
			}
		}
		printf("attribute size = %d\n", attributes.size());
	}

	void genPlaneIndices(float div) {
		
		printf("masuk ke plane indices\n");
		int n = div;
		for (int row = 0; row < n; row++) {
			for (int col = 0; col < n; col++) {
				int indx = col + (row * (div + 1));
				// top 										   ____		
				indices.push_back(indx);					// |  /
				indices.push_back(indx + (div + 1) + 1);	// | /
				indices.push_back(indx + (div + 1));		// |/
				// bottom
				indices.push_back(indx);					//   /|
				indices.push_back(indx + 1);				//  / |
				indices.push_back(indx + (div + 1) + 1);	// /__|
			}
		}
		printf("size indices: %d\n", indices.size());
	}
};

#endif
