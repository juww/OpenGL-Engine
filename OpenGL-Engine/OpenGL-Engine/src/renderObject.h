#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <glm/glm.hpp>
#include "material.h"

class RenderObject {
public:
    glm::mat4 matrix;
    std::shared_ptr<Materials> material;
    unsigned int vao, ebo;
    int drawMode, count, type;
    RenderObject(glm::mat4 pMatrix, unsigned int pVao, unsigned int pEbo, int pDrawMode, int pCount, int pType) :
        matrix(pMatrix), vao(pVao), ebo(pEbo), drawMode(pDrawMode), count(pCount), type(pType)
    {
    }

    RenderObject() {
        matrix = glm::mat4(1.0f);
        vao = ebo = 0;
        drawMode = count = type = 0;
    }

    ~RenderObject() {
    }

    void setMaterial(std::shared_ptr<Materials> pMaterial) {
        material = pMaterial;
    }
    
    RenderObject& operator=(const RenderObject& pRenderObject) {
        matrix = pRenderObject.matrix;
        vao = pRenderObject.vao;
        ebo = pRenderObject.ebo;
        count = pRenderObject.count;
        type = pRenderObject.type;
        material = pRenderObject.material;

        return *this;
    }
};

#endif // !RENDER_OBJECT_H
