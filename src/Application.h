#pragma once

#include "Camera.h"
#include "Model.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

class Application
{
public:
	Application(int height, int width, GLFWwindow* window);
	~Application();

	int getWindowHeight();
	int getWindowWidth();
	GLFWwindow* getWindow();

	void Initialize();
	void Update(float deltaTime);
	void Draw();

	bool SetupShadowMap();
	void CreateShadowMap();

	bool Initialize3DTextures(Texture3D& Texture, GLint textureCoding, GLint dataType);
	void CreateVoxels();
	void UpdateVoxels();
	void drawVoxels();

	void InitializeLighting(Shader * shader);

	void DebugInputs();

	void showShadowMapDebug(GLuint textureID);

	void computeShaderTest();

	void sparseTextureCommitment();

private:
	int windowWidth_;
	int windowHeight_;


	GLFWwindow* window_;
	Camera* camera_;

	Shader *defaultShader;
	Shader *voxelizationShader;
	Shader *shadowMapShader;
	Shader *visualizeVoxelsShader;
	Shader *shadowMapDebugShader;
	Shader *bounceShader;
	Shader *computeShader;
	Shader* lightInjectShader;
	Shader* voxelInitializaton;

	Model *model;

	//// Voxel Stuff
	GLuint voxelTextureSize = 256;
	glm::mat4 projX_, projY_, projZ_;
	const float voxelGridWorldSize_ = 200.0f;
	Texture3D voxelTexture_;
	Texture3D voxelTextureBounce_;
	Texture3D voxelNormalTexture_;
	GLuint quadVertexArray_;
	GLuint quadVBO_;


	// // Shadow Map Stuff
	GLuint depthFramebuffer_;
	Texture2D depthTexture_;
	glm::mat4 depthViewProjectionMatrix_;


	// // Light Stuff
	glm::vec3 lightDirection_ = glm::vec3(-1.5, 4, -0.25);


	// Math Stuff
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 modelViewMatrix;
	glm::mat4 depthModelViewProjectionMatrix;


	// Debugging inputs
	bool press1_ = false, press2_ = false, press3_ = false, press4_ = false;
	bool showDirectDiffuse = true;
	bool showIndirectDiffuse = true;
	bool showIndirectSpecular = true;
	bool showAmbientOcclusion = true;
	
	bool showVoxels = false;
	bool showMeshRender = true;
	bool showShadowMap = false;

	bool doubleBounce = true;
	bool newShadowMapNeeded = true;

	double averageFPS;
	double timeSinceStarted;

	struct PageUsageInfo {
		int baseLevel[512 * 512 * 512 / (32 * 32 * 32)];  // Adjust size based on your needs
		int mipLevel1[256 * 256 * 256 / (32 * 32 * 32)];
		int mipLevel2[128 * 128 * 128 / (32 * 32 * 32)];
		int mipLevel3[64 * 64 * 64 / (32 * 32 * 32)];
		int mipLevel4[32 * 32 * 32 / (32 * 32 * 32)];// Continue for each mip level needed
		// Other mip levels as necessary
	};
};

