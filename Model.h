#pragma once

#include <json/json.h>
#include "Mesh.h"
#include "Voxel_Cone_Tracing.h"

using json = nlohmann::json;

class Model
{
public:
	// Loads in a model from a file and stores tha information in 'data', 'JSON', and 'file'
	Model(const char* file);

	void CreateDepthTexture(Shader& shader, Camera& camera, glm::mat4 & depthViewProjectionMatrix_);
	void Draw(Shader& shader, Camera& camera, glm::mat4& depthViewProjectionMatrix_);

	void DrawVoxels(Shader& shader, Camera& camera, glm::mat4& depthViewProjectionMatrix_);

	void VisualizeVoxels(Shader& shader, Camera& camera);

	float Globalscale = 0.05f;
private:
	// Variables for easy access
	const char* file;
	std::vector<unsigned char> data;
	json JSON;

	// All the meshes and transformations
	std::vector<Mesh> meshes;
	std::vector<Voxelization> voxels;
	std::vector<glm::vec3> translationsMeshes;
	std::vector<glm::quat> rotationsMeshes;
	std::vector<glm::vec3> scalesMeshes;
	std::vector<glm::mat4> matricesMeshes;
	Texture3D * voxel_texture_ = nullptr;
	Voxel_Cone_Tracing * voxel_cone_tracing_ = nullptr;

	


	// Prevents textures from being loaded twice
	std::vector<std::string> loadedTexName;
	std::vector<Texture> loadedTex;

	// Loads a single mesh by its index
	void loadMesh(unsigned int indMesh);

	// Traverses a node recursively, so it essentially traverses all connected nodes
	void traverseNode(unsigned int nextNode, glm::mat4 matrix = glm::mat4(1.0f));

	// Gets the binary data from a file
	std::vector<unsigned char> getData();
	// Interprets the binary data into floats, indices, and textures
	std::vector<float> getFloats(json accessor);
	std::vector<GLuint> getIndices(json accessor);
	std::vector<std::vector<Texture>> getTextures();

	// Assembles all the floats into vertices
	std::vector<Vertex> assembleVertices
	(
		std::vector<glm::vec3> positions,
		std::vector<glm::vec3> normals,
		std::vector<glm::vec2> texUVs
	);

	// Helps with the assembly from above by grouping floats
	std::vector<glm::vec2> groupFloatsVec2(std::vector<float> floatVec);
	std::vector<glm::vec3> groupFloatsVec3(std::vector<float> floatVec);
	std::vector<glm::vec4> groupFloatsVec4(std::vector<float> floatVec);

	void loadTextureByIndex(int index, const char* type, const std::string& fileDirectory, std::vector<Texture>& textures, int slot);
	int findImageIndexByTextureIndex(int index);
};
