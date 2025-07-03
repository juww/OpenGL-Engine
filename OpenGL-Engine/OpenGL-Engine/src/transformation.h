#pragma once
#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Transformation {
public:
    glm::mat4 matrix;
    glm::mat4 inverseMatrix;

    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec4 rotate;
    glm::quat quaternion;

    Transformation() {
        matrix = glm::mat4(1.0f);
        inverseMatrix = glm::mat4(1.0f);

        pos = glm::vec3(0.0f);
        scale = glm::vec3(1.0f, 1.0f, 1.0f);
        rotate = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        quaternion = glm::quat();
    }

    Transformation& operator=(const Transformation& t) {
        matrix = t.matrix;
        inverseMatrix = t.inverseMatrix;

        pos = t.pos;
        scale = t.scale;
        rotate = t.rotate;
        quaternion = t.quaternion;

        return *this;
    }

    void localTransform() {
        // M = T * R * S
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

        quaternion = glm::quat(rotate.x, rotate.y, rotate.z, rotate.w);
        glm::mat4 R = glm::mat4_cast(quaternion);

        glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);

        matrix = T * R * S;
    }

    void inverseMatrices() {
        inverseMatrix = glm::inverse(matrix);
    }
};

#endif // !TRANSFORMATION_H