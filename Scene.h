#pragma once
#include <glad/glad.h>
#include <vector>
class Scene
{
public:
	Scene();
	std::vector<GLfloat> InsertVertices(std::vector<GLfloat> vertices, float xPos, float yPos, float r, float g, float b);

	std::vector<GLuint> InsertIndices(
		std::vector<GLuint> indices,
		GLuint topIndex,
		GLuint bottomLeftIndex,
		GLuint bottomRightIndex
	);

	std::vector<GLfloat> CircleVertices(
		std::vector<GLfloat> vertices,
		float radius,
		float x,
		float y,
		float r,
		float g,
		float b,
		int triangles
	);

	std::vector<GLuint> CircleIndices(
		std::vector<GLuint> indices,
		int triangles
	);

};

