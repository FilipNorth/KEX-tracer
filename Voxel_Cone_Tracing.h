#pragma once

#include "Mesh.h"

class Voxel_Cone_Tracing
{
public: 
	Voxel_Cone_Tracing();


	void visualizeVoxels(
		Shader& voxelShader, // This is the shader that will handle the voxelization
		Texture3D* voxelTexture, // ID of the 3D texture to store voxel data
		Camera& camera
	);

};

