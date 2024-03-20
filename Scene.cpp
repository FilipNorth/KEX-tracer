#include "Scene.h"
#include <corecrt_math.h>
#include <cmath>

Scene::Scene() {

}

std::vector<GLfloat> Scene::InsertVertices(std::vector<GLfloat> vertices, float xPos, float yPos, float r, float g, float b) {
    vertices.insert(vertices.end(), { xPos, yPos, 0.0f, r, g, b });

    return vertices;
}

std::vector<GLuint> Scene::InsertIndices(std::vector<GLuint> indices, GLuint topIndex, GLuint bottomLeftIndex, GLuint bottomRightIndex) {
    indices.insert(indices.end(), { topIndex, bottomLeftIndex, bottomRightIndex });
    return indices;
}

std::vector<GLfloat> Scene::CircleVertices(std::vector<GLfloat> vertices, float radius, GLfloat xPos, GLfloat yPos, float r, float g, float b, int triangles) {
    vertices.clear();
    const GLfloat PI = 3.14159f;
    GLfloat angleIncrement = 2 * PI / triangles;
    // Center vertex
    vertices.push_back(xPos);
    vertices.push_back(yPos);
    vertices.push_back(0.0f); // Assuming Z=0 for 2D circle

    vertices.push_back(r);
    vertices.push_back(g);
    vertices.push_back(b);

    for (int i = 0; i <= triangles; ++i) {
        GLfloat angle = i * angleIncrement;
        GLfloat x = xPos + radius * cos(angle);
        GLfloat y = yPos + radius * sin(angle);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f); // Z=0 for 2D circle

        vertices.push_back(r * i / triangles);
        vertices.push_back(g * i / triangles);
        vertices.push_back(b * i / triangles);
    }

    return vertices;

}

std::vector<GLuint> Scene::CircleIndices(std::vector<GLuint> indices, int triangles) {
    indices.clear();
    // Generate indices for each triangle
    int size = indices.size();
    for (int i = 1; i <= triangles; ++i) {
        indices.push_back(size); // Center vertex
        indices.push_back(i + size);
        indices.push_back(i + 1 + size);
    }
    // Fix the last triangle's last index to loop back to the first vertex
    indices.back() = size + 1;

    return indices;
}