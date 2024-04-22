#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "shaderClass.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Function prototypes
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
std::string loadShaderSource(const char* filename);
GLuint compileShader(const char* file, GLenum type);

// Main function
int main1() {
    // Initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Voxelization", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Compile shaders
    Shader shader("Shaders/voxelTesting.vert", "Shaders/voxelTesting.geom", "Shaders/voxelTesting.frag");

    // Check max geometry output vertices
    GLint maxGeometryOutputVertices;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxGeometryOutputVertices);
    std::cout << "Max Geometry Output Vertices: " << maxGeometryOutputVertices << std::endl;

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, shader.program);
    glLinkProgram(shaderProgram);

    // Set up vertex data and buffers
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.0f,  0.5f, -0.5f
    };
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

std::string loadShaderSource(const char* filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    if (file) {
        buffer << file.rdbuf();
        file.close();
        return buffer.str();
    }
    std::cerr << "Error loading shader: " << filename << std::endl;
    return "";
}

GLuint compileShader(const char* file, GLenum type) {
    std::string src = loadShaderSource(file);
    GLuint shader = glCreateShader(type);
    const char* shaderSrc = src.c_str();
    glShaderSource(shader, 1, &shaderSrc, nullptr);
    glCompileShader(shader);
    return shader;
}