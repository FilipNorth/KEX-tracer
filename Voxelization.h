#pragma once
#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Texture.h"
//#include "Texture3D.h"
#include "Scene.h"
#include "Libraries/include/glm/glm.hpp"	

class Voxelization
{
public:

	struct Texture2D {
		GLuint textureID;
		int width, height, componentsPerPixel;
	};

	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;

	VAO VAO;

	Voxelization(std::vector <Vertex>& vertices,
		std::vector <GLuint>& indices,
		std::vector <Texture>& textures);



	void Draw
	(
		Shader& voxelShader, // This is the shader that will handle the voxelization
		Texture3D *voxelTexture, // ID of the 3D texture to store voxel data
		Camera& camera, 
		glm::mat4 modelMatrix = glm::mat4(1.0f)
	);
	void visualizeVoxels(
		Shader& voxelShader, // This is the shader that will handle the voxelization
		Texture3D* voxelTexture, // ID of the 3D texture to store voxel data
		Camera& camera
	);

	void createShadowMap();

private:
	Shader * voxelizationShader;
	GLuint voxelTextureSize = 128; 
	const GLuint program = 0;
	GLuint depthFramebuffer_;
	Texture2D depthTexture_;
	GLuint shadowShader_;
	glm::mat4 depthViewProjectionMatrix_;
	glm::vec3 lightDirection_ = glm::vec3(-0.3, 0.9, -0.25);

};

