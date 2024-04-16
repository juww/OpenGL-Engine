#pragma once
#ifndef INTERPOLATE_H
#define INTERPOLATE_H

namespace interpolate {

    template<class T>
    T lerp(T a, T b, float t) {
        return a + (t * (b - a));
    }

    template<class T>
    T inverseLerp(T a, T b, float t) {
        return (t - a) / (b - a);
    }


    glm::vec4 slerp(glm::vec4 v0, glm::vec4 v1, float t) {

        // use normalize quaternion, calculation become wrong (result is nan);
        //v0 = glm::normalize(v0);
        //v1 = glm::normalize(v1);
        float dotq = glm::dot(v0, v1);
        if (dotq < 0) {
            v1 = -v1;
            dotq = glm::dot(v0, v1);
        }
        float angle = glm::acos(dotq);
        float sinAngle = glm::sin(angle);

        if (sinAngle == 0) {
            return interpolate::lerp(v0, v1, t);
        }

        glm::vec4 q0 = (sin((1.0f - t) * angle) / sinAngle) * v0;
        glm::vec4 q1 = (sin(t * angle) / sinAngle) * v1;

        return q0 + q1;
    }

}

#endif // !INTERPOLATE_H
