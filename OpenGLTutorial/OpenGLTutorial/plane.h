#pragma once
#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include <vector>

class Vertice {
public:
	int id;
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

class Plane {

public:

	int width;
	int length;
	float spacing;
	std::vector<Vertice> vertices;
	// use x and z; y height
	Plane(const int& w, int& l, float& space) {
		vertices.reserve(w * l);
		spacing = space;
		for (int r = 0; r < width; r++) {
			for (int c = 0; c < length; c++) {

			}
		}
	}
	~Plane() {

	}

private:

	



};

#endif