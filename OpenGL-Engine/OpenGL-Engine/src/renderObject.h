#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <glm/glm.hpp>

class RenderObject {
public:
    glm::mat4 model;
    unsigned int vao, ebo, count, type;
    RenderObject(glm::mat4 pModel, unsigned int pVao, unsigned int pEbo, unsigned int pCount, unsigned int pType) :
        model(pModel), vao(pVao), ebo(pEbo), count(pCount), type(pType) 
    {
    }

    RenderObject() {
        model = glm::mat4(1.0f);
        vao = ebo = count = type = 0;
    }

    ~RenderObject() {
    }
    
    RenderObject& operator=(const RenderObject& pRenderObject) {
        model = pRenderObject.model;
        vao = pRenderObject.vao;
        ebo = pRenderObject.ebo;
        count = pRenderObject.count;
        type = pRenderObject.type;

        return *this;
    }
};

#endif // !RENDER_OBJECT_H
