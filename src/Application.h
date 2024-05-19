#pragma once

#include "Camera.h"
#include "Model.h"

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
	void drawVoxels();
	void CreateAdditionalBounces();

	void InitializeLighting(Shader * shader);

	void DebugInputs();

	void showShadowMapDebug(GLuint textureID);

	void computeShaderTest();

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
	glm::vec3 lightDirection_ = glm::vec3(-1.5, 1.9, -0.25);


	// Math Stuff
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;


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
};
