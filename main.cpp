
#define __USE_MINGW_ANSI_STDIO 0
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "camera.h"
#include "Surface.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const * path);
unsigned int loadVAO(unsigned int sizeVertices, glm::vec3* firstVertex, unsigned int sizeEBO, unsigned int* firstEBO);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 1024;
bool wireframeMode = false;
bool colataMode = true;
bool curtiMode = false;
bool albanoMode = false;
Surface* surface;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 45.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float factorTimeSpeedCamera = 2.0f;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Surface colata("./data/altitudes.dat","./data/lava.dat", "./data/temperature.dat");
    colata.loadTexture("./textures/surface.png");
    colata.VAO = loadVAO(colata.vertices.size(), &colata.vertices[0], colata.indicesEBO.size(), &colata.indicesEBO[0]);

    Surface albano("./data/DEM_Albano.asc");
    albano.loadTexture("./textures/white.png");
    albano.VAO = loadVAO(albano.vertices.size(), &albano.vertices[0], albano.indicesEBO.size(), &albano.indicesEBO[0]);

    Surface curti("./data/DEM_Curti.asc");
    curti.loadTexture("./textures/white.png");
    curti.VAO = loadVAO(curti.vertices.size(), &curti.vertices[0], curti.indicesEBO.size(), &curti.indicesEBO[0]);    
    
    Shader surfaceShader("./shader/surface.vs", "./shader/surface.fs");

    surfaceShader.use();
    surfaceShader.setVec3("light.ambient", 0.3f, 0.3f, 0.3f);
    surfaceShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
    surfaceShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    surfaceShader.setFloat("light.constant", 1.0f);
    surfaceShader.setFloat("light.linear", 0.00007);
    surfaceShader.setFloat("light.quadratic", 0.00000035);

    surface = &colata;

    
    //camera.setMovementSpeed(std::max(surface.getRows(), surface.getColumns()) * surface.getCellSize()/factorTimeSpeedCamera);

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 40000.0f);

        surfaceShader.use();
        surfaceShader.setMat4("projection",projection);
        surfaceShader.setMat4("view",view);
        surfaceShader.setVec3("light.position", camera.Position);
        surfaceShader.setVec3("viewPos", camera.Position);

        glm::mat4 model = glm::mat4();
        if(wireframeMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if(colataMode)
        {
            surface = &colata;
        }
        else if(curtiMode)
        {
            surface = &curti;
        }
        else if(albanoMode)
        {
            surface = &albano;
        }

        model = glm::translate(model, glm::vec3(0.0f,-surface->getDropHeight(),0.0f));
        surfaceShader.setMat4("model",model);

        //set camera speed accordingly to the scene
        camera.setMovementSpeed(std::max(surface->getRows(), surface->getColumns()) * surface->getCellSize()/factorTimeSpeedCamera);
        glBindVertexArray(surface->VAO); 
        glBindTexture(GL_TEXTURE_2D, surface->texture);
        glDrawElements(GL_TRIANGLES, surface->indicesEBO.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow *window)
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

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        wireframeMode=!wireframeMode;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        colataMode=true;
        curtiMode=false;
        albanoMode=false;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        colataMode=false;
        curtiMode=true;
        albanoMode=false;
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        colataMode=false;
        curtiMode=false;
        albanoMode=true;
    }
}


void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadVAO(unsigned int sizeVertices, glm::vec3* firstVertex, unsigned int sizeEBO, unsigned int* firstEBO)
{
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizeVertices, firstVertex, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * sizeEBO, firstEBO, GL_STATIC_DRAW);

    //Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    //Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*) ( 3 * sizeof(GLfloat) ));
    glEnableVertexAttribArray(1);

    //Red value
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*) ( 6 * sizeof(GLfloat) ));
    glEnableVertexAttribArray(2);

    //Texture coordinate
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*) ( 9 * sizeof(GLfloat) ));
    glEnableVertexAttribArray(3);

    return VAO;
}