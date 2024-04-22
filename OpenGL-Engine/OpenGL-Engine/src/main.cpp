#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader_m.h"
#include "camera.h"
#include "skybox.h"
#include "loadModel.h"
#include "plane.h"
#include "light.h"
#include "GUI.h"

#include <iostream>
#include <string>
#include <cstring>
#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tinyGLTF/tiny_gltf.h"

#define INPUTFILE  freopen("input.txt","r",stdin)
#define OUTPUTFILE freopen("output.txt","w",stdout)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorcb = true;
bool updatecb = false;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    std::vector<Light> lights;
    Light l1;
    l1.setLight(glm::vec3(-0.2f, -1.0f, -0.3f));
    lights.push_back(l1);
    l1.setLight(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.0f, 1.0f);
    lights.push_back(l1);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction 

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    // build and compile our shader zprogram
    // ------------------------------------
    Shader cubeShader("cube.vs", "cube.fs", "normalMapping.gs");
    Shader normalLineShader("normalLine.vs", "normalLine.fs", "normalLine.gs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");
    Shader carafeShader("cube.vs", "cube.fs", "normalMapping.gs");
    Shader planeShader("plane.vs", "plane.fs");
    Shader grassShader("grass.vs", "grass.fs");
    // planeShader.setTessellationShader("TessellationControlShader.tcs", "TessellationEvaluationShader.tes");
    //Shader skeletalModel("skeletal.vs", "skeletal.fs", "skeletal.gs");

    const std::string& pathfile = "res/models/simpleSkin/scene.gltf";
    //const std::string& pathfile = "assets/models/phoenix_bird/scene.gltf";
    loadModel carafe(pathfile.c_str());

    carafe.animator.doAnimation(0);

    Skybox skybox;

    // first, configure the cube's VAO (and VBO)
    unsigned int cubeVBO;
    //glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    int np = 65, lvl = 1;
    Plane plane(np);
    //plane.GenerateNoiseMap(np, np, 4, 27.9f, 4, 0.5f, 2.0f, offset);
    plane.InitTerrainChunk(lvl, 64.0f, camera.Position);
    plane.initGrass(50, grassShader);
    plane.setAllUniform(planeShader);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    cubeShader.use();
    cubeShader.setInt("albedoMap", 0);
    cubeShader.setInt("normalMap", 1);
    cubeShader.setInt("roughnessMap", 2);

    carafeShader.use();
    carafeShader.setInt("albedoMap", 0);
    carafeShader.setInt("normalMap", 1);
    carafeShader.setInt("roughnessMap", 2);

    
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // use it for look the all the line of object
     //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //carafe.DrawSkeleton(carafeShader);

    float frameCount = 0.0f;
    float prevFrame = static_cast<float>(glfwGetTime());

    GUI::initialize(window);
    // parameter for generate terrain
    GUI::terrainParam tp();

    int seed = 4;
    float scale = 27.9f;
    int octaves = 4;
    float persistence = 0.5f;
    float lacunarity = 2.0f;
    glm::vec2 offset(0.0f, 0.0f);
    float heightMultiplier = 5.0f;

    // grass
    float frequency = 3.0f;
    float amplitude = 0.5f;
    float scl = 1.12f;
    float drp = 0.7f;

    plane.grass.generateNoiseMap(grassShader, 1, 10.0f, 4, 1.5f, 2.0f, { 0.0f,0.0f });
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        frameCount += 1.0f;
        deltaTime = currentFrame - lastFrame;
        
        if (currentFrame - prevFrame >= 1.0) {
            printf("frame per second : %f\n", frameCount);
            printf("%f ms\n", 1000.0f / frameCount);
            printf("deltaTime = %f \n", deltaTime);
            frameCount = 0.0f;
            prevFrame = currentFrame;
            updatecb = true;
        }
        lastFrame = currentFrame;

        GUI::GUIFrame();
        GUI::modelTransform(carafe.pos, carafe.rot, carafe.angle, carafe.scale);
        GUI::modelAnimation(carafe.animator.animations.size(), 0);
        bool changeParam = GUI::proceduralTerrainParam(seed, scale, octaves, persistence, lacunarity, offset, heightMultiplier);
        GUI::grassParam(frequency, amplitude, scl, drp);

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);

        glBindVertexArray(0);
        // load carafe
        carafeShader.use();
        int light_n = lights.size();
        carafeShader.setInt("light_n", light_n);
        for (int i = 0; i < light_n; i++) {
            carafeShader.setInt("lightID[" + std::to_string(i) + "]", i+1);
            carafeShader.setVec3("lightDirection[" + std::to_string(i) + "]", lights[i].m_Direction);
            carafeShader.setVec3("lightPosition[" + std::to_string(i) + "]", lights[i].m_Position);
            carafeShader.setVec3("lightColor[" + std::to_string(i) + "]", lights[i].m_Color);
        }

        carafeShader.setVec3("viewPos", camera.Position);
        carafeShader.setMat4("projection", projection);
        carafeShader.setMat4("view", view);

        carafe.update(carafeShader, deltaTime);
        carafe.DrawModel(carafeShader);

        // draw plane
        plane.update(camera.Position, seed, scale, octaves, persistence, lacunarity, offset, heightMultiplier, changeParam);
        plane.draw(planeShader, projection, view, np, camera.Position);
        plane.drawGrass(grassShader, view, projection, currentFrame, frequency, amplitude, scl, drp);

        //plane.drawNormalLine(normalLineShader, projection, view, np, camera.Position);

        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, { 1.0f, 0.0f, 0.0f });
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        lightCubeShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        glm::mat4 viewStatic = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", viewStatic);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skybox.vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        ImGui::End();


        GUI::renderUI();
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    GUI::shutDown();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &cubeVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!updatecb) return;
        updatecb = false;
        if (!cursorcb) {
            cursorcb = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            cursorcb = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
    if(cursorcb) camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
