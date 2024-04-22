#pragma once
#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Texture.h"
#include "Texture3D.h"
#include "Scene.h"

class Voxelization
{
public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;

	VAO VAO;

	Voxelization(std::vector <Vertex>& vertices,
		std::vector <GLuint>& indices,
		std::vector <Texture>& textures, Texture3D &voxelTexture);



	void Draw
	(
		Shader& shader,
		Camera& camera,
		glm::mat4 matrix = glm::mat4(1.0f),
		glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f)

	);

private:
	Shader * voxelizationShader;
	GLuint voxelTextureSize = 64; 
	Texture3D voxelTexture;
	const GLuint program = 0;



};

