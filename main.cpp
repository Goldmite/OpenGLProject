#include <glad/glad.h> 
#include <GLFW\glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader.h"

#include <iostream>
#include <math.h>
#include <time.h> 

#define PI 3.1415926535
// settings
const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 800;

const int AMOUNT_OF_BOIDS = 1600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
int main()
{
    //glfw initialization and configuration
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //create glfw window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGLProject", NULL, NULL);
    if (window == NULL)
    {
        std::cout<< "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    // glad load all OpenGl function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    //build and compile the shader program
    GeneralShader ourShader("Shaders/vertex.glsl", "Shaders/fragment.glsl");
    ComputeShader computeShader("Shaders/compute.glsl");
    struct boid {
        float x = 0;
        float y = 0;
        float angle = 0;
        int turndelay = 0;
        int spawndelay = 0;
        int randomAngle = 0;
    };
    boid boidArray[AMOUNT_OF_BOIDS];
    for (int i = 0; i < AMOUNT_OF_BOIDS; i++) {
        boid Bob;
        Bob.x = rand() % (WINDOW_WIDTH);
        Bob.y = rand() % (WINDOW_HEIGHT);
        Bob.angle = i;
        Bob.turndelay = 0;
        Bob.spawndelay = 0;
        Bob.randomAngle = rand() % 1000000;
        boidArray[i] = Bob;
    }
    //3 dots for triangle
    float vertices[] {
        //positions         //texture coords
        1.0f,  1.0f, 0.0f,  1.0f, 1.0f,   //top right
        1.0f, -1.0f, 0.0f,  1.0f, 0.0f,   //bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,   //bottom left
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f    //top left 
    };
    //fullscreen texture made of 2 triangles
    unsigned int indices[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };
    unsigned int VertexBuffer, VertexArray, ElementBuffer, maintexture, SSBO; //objects
    
    glGenBuffers(1, &VertexBuffer);
    glGenVertexArrays(1, &VertexArray);
    glGenBuffers(1, &ElementBuffer);
    glGenBuffers(1, &SSBO);
    //bind vertex array object
    glBindVertexArray(VertexArray);
    //copies user vertex data into buffer's memory
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //bind elements
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    //bind ssbo
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, AMOUNT_OF_BOIDS * sizeof(boid), boidArray, GL_DYNAMIC_READ);
    //telling OpenGL how to interpret vertex data / per attribute
    // location | size of attribute | type of data | normalize data (no) | space between consecutive vertex attributes | offset where the position begins in buffer
    //position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //texture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //load and create texture
    glGenTextures(1, &maintexture);
    glBindTexture(GL_TEXTURE_2D, maintexture); 
     //set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //load image, create texture and generate mipmaps
    //empty texture window
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    //activate shader
   

    //unbind
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //render loop
    while(!glfwWindowShouldClose(window))
    {
        //input
        processInput(window);
        //rendering commands
        //clear colour buffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //shaders
        computeShader.use();
        float TimeValue = glfwGetTime();
        glUniform1f(0, TimeValue);

        glUniform1i(0, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, maintexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SSBO); //ssbo
        
        computeShader.dispatch(AMOUNT_OF_BOIDS / 16, 1); 
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        ourShader.use();
        glBindImageTexture(1, maintexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        //activate then bind textures
        glBindVertexArray(VertexArray);
        //glBindImageTexture(0, maintexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, maintexture);
        
        //draw shape
        
        //draw thingie
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        //glfw swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }
    // terminate, clearing all perviously allocated glfw resources
    glfwTerminate();
    return 0;
}
