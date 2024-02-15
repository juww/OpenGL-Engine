#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"
#include "camera.h"
#include "skybox.h"
#include "loadModel.h"
#include "plane.h"

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
const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
struct Lightnings {
    int ID;

    glm::vec3 direction;
    glm::vec3 position;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 color;

    Lightnings(int id, glm::vec3 dir, glm::vec3 pos, float cutoff, float outer, float c, float l, float q, glm::vec3 col) {
        ID = id;
        direction = dir;
        position = pos;
        cutOff = cutoff;
        outerCutOff = outer;
        constant = c;
        linear = l;
        quadratic = q;
        color = col;
    }
};

int main()
{
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
    if (window == NULL)
    {
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
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::vector<Lightnings> lights;
    lights.push_back({ 1,glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f) });
    lights.push_back({ 2,glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f) });

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
    //Shader skeletalModel("skeletal.vs", "skeletal.fs", "skeletal.gs");

    const std::string& pathfile = "assets/models/simpleSkin/scene.gltf";
    //const std::string& pathfile = "assets/models/phoenix_bird/scene.gltf";
    loadModel carafe(pathfile.c_str());

    Skybox skybox;

    // first, configure the cube's VAO (and VBO)
    unsigned int cubeVBO;
    //glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //glBindVertexArray(cubeVAO);

    //// position attribute
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(0);
    //// normal attribute
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);

    int np = 128;
    glm::vec2 offset(0.0f, 0.0f);
    Plane plane(np);
    plane.GenerateNoiseMap(np, np, 4, 27.9f, 4, 0.5f, 2.0f, offset);

    planeShader.use();
    planeShader.setInt("noiseMap", 0);

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
    // cubeShader.setInt("ambientOcclusionMap", 3);
    // cubeShader.setInt("metallicMap", 3);
    //cubeShader.setFloat("uMetallic", modelMesh.metallic);

    carafeShader.use();
    carafeShader.setInt("albedoMap", 0);
    carafeShader.setInt("normalMap", 1);
    carafeShader.setInt("roughnessMap", 2);
    carafe.animator.doAnimation(0);
    // cubeShader.setInt("ambientOcclusionMap", 3);
    //carafeShader.setFloat("uMetallic", modelMesh.metallic);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // use it for look the all the line of object
     //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //carafe.DrawSkeleton(carafeShader);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

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

        // be sure to activate shader when setting uniforms/drawing objects
        //cubeShader.use();
        //cubeShader.setVec3("light.position", lightPos);
        //cubeShader.setVec3("viewPos", camera.Position);
        //cubeShader.setVec3("light.color", lightColor);
        //cubeShader.setMat4("projection", projection);
        //cubeShader.setMat4("view", view);

        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, modelMesh.texColor);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, modelMesh.texNormal);
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, modelMesh.texRoughness);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, { 5.0f, -3.0f, 0.0f });
        //glm::mat4 rotMatrix = glm::mat4_cast(quaternion);
        //model = model * rotMatrix;
        //model = glm::rotate(model, (float)glfwGetTime(), { 1.0f,0.0f,0.0f });
        //model = glm::scale(model, {1.0f,1.0f,1.0f});
        //cubeShader.setMat4("model", model);
        // render the cube
        /*glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);*/
        //glBindVertexArray(modelMesh.vao);
        //modelMesh.DrawMesh();

        glBindVertexArray(0);
        // load carafe
        carafeShader.use();
        int light_n = lights.size();
        carafeShader.setInt("light_n", light_n);
        for (int i = 0; i < light_n; i++) {
            carafeShader.setInt("lightID[" + std::to_string(i) + "]", lights[i].ID);
            carafeShader.setVec3("lightDirection[" + std::to_string(i) + "]", lights[i].direction);
            carafeShader.setVec3("lightPosition[" + std::to_string(i) + "]", lights[i].position);
            carafeShader.setVec3("lightColor[" + std::to_string(i) + "]", lights[i].color);
        }

        carafeShader.setVec3("viewPos", camera.Position);
        carafeShader.setMat4("projection", projection);
        carafeShader.setMat4("view", view);

        carafe.update(carafeShader, deltaTime);
        carafe.DrawModel(carafeShader);

        // use this for show the normal line;
        //normalLineShader.use();
        //normalLineShader.setMat4("projection", projection);
        //normalLineShader.setMat4("view", view);
        //normalLineShader.setMat4("model", model);
        //glBindVertexArray(modelMesh.vao);
        //modelMesh.DrawMesh();

        // WIP: don't know how to draw the skeleton matrix
        //skeletalModel.use();
        //skeletalModel.setMat4("projection", projection);
        //skeletalModel.setMat4("view", view);
        //skeletalModel.setMat4("model", model);
        //carafe.DrawSkeleton(skeletalModel);

        //carafe.DrawModel(carafeShader);

        // draw plane
        plane.draw(planeShader, projection, view, model, np);

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

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
void processInput(GLFWwindow* window)
{
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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
