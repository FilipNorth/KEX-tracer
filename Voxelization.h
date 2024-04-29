#pragma once
#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Texture.h"
#include "Texture3D.h"
#include "Scene.h"
#include "Libraries/include/glm/glm.hpp"	

class Voxelization
{
public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;

	VAO VAO;

	Voxelization(std::vector <Vertex>& vertices,
		std::vector <GLuint>& indices,
		std::vector <Texture>& textures, Texture3D *voxelTexture);



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

private:
	Shader * voxelizationShader;
	GLuint voxelTextureSize = 128; 
	Texture3D voxelTexture;
	const GLuint program = 0;



};

