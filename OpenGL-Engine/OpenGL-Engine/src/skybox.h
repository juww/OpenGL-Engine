#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "tinyGLTF/stb_image.h"
#include "JoeyDeVries/filesystem.h"

#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

const float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};


const std::string path = "res/textures/cubemap/SaintPetersBasilica/";
const std::vector<std::string> faces
{
    FileSystem::getPath("res/skybox/right.jpg"),
    FileSystem::getPath("res/skybox/left.jpg"),
    FileSystem::getPath("res/skybox/top.jpg"),
    FileSystem::getPath("res/skybox/bottom.jpg"),
    FileSystem::getPath("res/skybox/front.jpg"),
    FileSystem::getPath("res/skybox/back.jpg")
    
    //FileSystem::getPath(path + "posx.jpg"),
    //FileSystem::getPath(path + "negx.jpg"),
    //FileSystem::getPath(path + "posy.jpg"),
    //FileSystem::getPath(path + "negy.jpg"),
    //FileSystem::getPath(path + "posz.jpg"),
    //FileSystem::getPath(path + "negz.jpg")

    //FileSystem::getPath(path + "cubemap0.bmp"),
    //FileSystem::getPath(path + "cubemap1.bmp"),
    //FileSystem::getPath(path + "cubemap2.bmp"),
    //FileSystem::getPath(path + "cubemap3.bmp"),
    //FileSystem::getPath(path + "cubemap4.bmp"),
    //FileSystem::getPath(path + "cubemap5.bmp"),
};

class Skybox {
public:
    unsigned int vao, vbo;
    unsigned int cubemapTexture;
    int width, height, nrChannels;

    Skybox() {
        printf("generate skybox\n");
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glGenTextures(1, &cubemapTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        int _width, _height, _nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++) {
            unsigned char* data = stbi_load(faces[i].c_str(), &_width, &_height, &_nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        width = _width;
        height = _height;

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    void draw(Shader* shader, const glm::mat4& projection, const glm::mat4& viewStatic) {

        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        shader->use();
        //glm::mat4 viewStatic = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        shader->setMat4("view", viewStatic);
        shader->setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

    }
};

#endif