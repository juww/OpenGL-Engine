#pragma once
#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include <vector>

class Plane {

public:

	int width;
	int length;
	float spacing;
	
	std::vector<float> attributes;
	std::vector<unsigned int> indices;

	// use x and z; y height
	Plane(const int& w, const int& l, const float& space) {

		width = w;
		length = l;
		spacing = space;

		attributes = planeVertices();

	}

	~Plane() {

	}

private:

	std::vector<float> planeVertices(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, float div, int width, int lenght) {
		std::vector<float> plane;
												// v3-----v2
		glm::vec3 vec03 = (v3 - v0) / div;		// |       |
		glm::vec3 vec12 = (v2 - v1) / div;		// |       |
												// v0-----v1
		const int n = div + 1;
		for (int row = 0; row < n; row++) {
			glm::vec3 start = v0 + vec03 * (float)row;
			glm::vec3 end = v1 + vec12 * (float)row;
			glm::vec3 vectorDiv = (end - start) / div;
			for (int col = 0; col < n; col++) {
				glm::vec3 line = start + vectorDiv * (float)col;
				// position
				plane.push_back(line.x);
				plane.push_back(line.y);
				plane.push_back(line.z);

				// texture / UV
				plane.push_back(float(col) / div);
				plane.push_back(float(row) / div);

				// Tangent space Vectors;
				glm::vec3 tangent = glm::normalize(vectorDiv);
				glm::vec3 nextStart = start + vec03;
				glm::vec3 nextEnd = end + vec12;
				glm::vec3 nextVectorDiv = (nextEnd - nextStart) / div;
				glm::vec3 nextRowCrntVec = nextStart + nextVectorDiv * float(col);
				glm::vec3 possibleBitangent = glm::normalize(nextRowCrntVec - line);

				// normal
				glm::vec3 normal = glm::cross(tangent, possibleBitangent);
				plane.push_back(normal.x);
				plane.push_back(normal.y);
				plane.push_back(normal.z);
				// tangent
				plane.push_back(tangent.x);
				plane.push_back(tangent.y);
				plane.push_back(tangent.z);
				// Bitangent
				// The Bitangent could also be the opposite of this
				plane.push_back(possibleBitangent.x);
				plane.push_back(possibleBitangent.y);
				plane.push_back(possibleBitangent.z);
			}
		}

		return plane;
	}
};

#endif
