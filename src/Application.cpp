#include "Application.h"


Application::Application(int height, int width, GLFWwindow * window)
{
	windowHeight_ = height;
	windowWidth_ = width;
	window_ = window;
	camera_ = NULL;

}

Application::~Application()
{
	delete camera_;
}

void Application::Initialize() {
	std::cout << "Initializing application" << "\n";

	camera_ = new Camera(windowWidth_, windowHeight_, glm::vec3(1.0f, 2.0f, 3.0f));
	//camera_->Orientation = glm::vec3(0, 1, 0);
	
	// Load shaders
	defaultShader = new Shader("../../../../Shaders/default.vert",  "../../../../Shaders/default.frag");
	bounceShader = new Shader("../../../../Shaders/default.vert", "../../../../Shaders/SaveAdditionalBounce.frag");
	visualizeVoxelsShader = new Shader("../../../../Shaders/renderVoxels.vert", "../../../../Shaders/renderVoxels.geom", "../../../../Shaders/renderVoxels.frag");
	shadowMapShader = new Shader("../../../../Shaders/shadowMap.vert", "../../../../Shaders/shadowMap.frag");
	voxelizationShader = new Shader("../../../../Shaders/voxelization.vert", "../../../../Shaders/voxelization.geom", "../../../../Shaders/voxelization.frag");
	voxelInitializaton = new Shader("../../../../Shaders/voxelization.vert", "../../../../Shaders/voxelization.geom", "../../../../Shaders/voxelizationStart.frag");
	shadowMapDebugShader = new Shader("../../../../Shaders/shadowMapDebug.vert", "../../../../Shaders/shadowMapDebug.frag");

	lightInjectShader = new Shader("../../../../Shaders/default.vert", "../../../../Shaders/directLightVoxel.frag");

	computeShader = new Shader("../../../../Shaders/lightBounces.comp");
	
	// Load model
	std::cout << "Loading model" << "\n";

	double startTime = glfwGetTime();
	model = new Model("../../../../Models/Sponza-glTF/Sponza.gltf");
	std::cout <<  glfwGetTime() - startTime << " Time taken to load model \n";
	std::cout << "Model loaded" << "\n";

	/// Initialize Shadow Map ///
	SetupShadowMap();


	/// Initialize 3D Textures ///
	Initialize3DTextures(voxelTexture_, GL_RGBA8, GL_UNSIGNED_BYTE);
	Initialize3DTextures(voxelTextureBounce_, GL_RGBA8, GL_UNSIGNED_BYTE);
	Initialize3DTextures(voxelNormalTexture_, GL_RGBA16F, GL_FLOAT);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Quad FBO
	static const GLfloat quad[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &quadVBO_);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glGenVertexArrays(1, &quadVertexArray_);

	timeSinceStarted = glfwGetTime();
	averageFPS = 0;



	// Activate voxel shader
	glUseProgram(voxelInitializaton->ID);

	// Set uniforms
	glUniform1i(glGetUniformLocation(voxelInitializaton->ID, "VoxelDimensions"), voxelTexture_.size);
	glUniformMatrix4fv(glGetUniformLocation(voxelInitializaton->ID, "ProjX"), 1, GL_FALSE, &projX_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelInitializaton->ID, "ProjY"), 1, GL_FALSE, &projY_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelInitializaton->ID, "ProjZ"), 1, GL_FALSE, &projZ_[0][0]);

	// Bind depth texture
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, depthTexture_.textureID);
	glUniform1i(glGetUniformLocation(voxelInitializaton->ID, "ShadowMap"), 5);

	// Bind single level of texture to image unit so we can write to it from shaders
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindImageTexture(0, voxelTexture_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUniform1i(glGetUniformLocation(voxelInitializaton->ID, "VoxelTexture"), 0);

	// Bind single level of texture to image unit so we can write to it from shaders
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindImageTexture(2, voxelNormalTexture_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glUniform1i(glGetUniformLocation(voxelInitializaton->ID, "VoxelNormalTexture"), 2);

	// activate voxel update shader

		// Activate voxel shader
	glUseProgram(voxelizationShader->ID);

	// Set uniforms
	glUniform1i(glGetUniformLocation(voxelizationShader->ID, "VoxelDimensions"), voxelTexture_.size);
	glUniformMatrix4fv(glGetUniformLocation(voxelizationShader->ID, "ProjX"), 1, GL_FALSE, &projX_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelizationShader->ID, "ProjY"), 1, GL_FALSE, &projY_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelizationShader->ID, "ProjZ"), 1, GL_FALSE, &projZ_[0][0]);

	// Bind depth texture
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, depthTexture_.textureID);
	glUniform1i(glGetUniformLocation(voxelizationShader->ID, "ShadowMap"), 5);

	// Bind single level of texture to image unit so we can write to it from shaders
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindImageTexture(0, voxelTexture_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUniform1i(glGetUniformLocation(voxelizationShader->ID, "VoxelTexture"), 0);

	// Activate compute shader

	glm::mat4 viewMatrix = camera_->view;
	glm::mat4 projectionMatrix = camera_->projection;


	glUseProgram(computeShader->ID);

	glBindImageTexture(1, voxelTextureBounce_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUniform1i(glGetUniformLocation(computeShader->ID, "bounceTexture"), 1);
	//glBindImageTexture(1, voxelTexture_.textureID, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);


	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, voxelTexture_.textureID);
	glUniform1i(glGetUniformLocation(computeShader->ID, "VoxelTexture"), 0);



	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_3D, voxelNormalTexture_.textureID);
	glUniform1i(glGetUniformLocation(computeShader->ID, "VoxelNormalTexture"), 2);


	startTime = glfwGetTime();
	CreateShadowMap();
	std::cout << glfwGetTime() - startTime << " Time taken to create shadowMap \n";

	startTime = glfwGetTime();
	CreateVoxels();
	std::cout << glfwGetTime() - startTime << " Time taken to create voxels \n";

	startTime = glfwGetTime();
	computeShaderTest();
	std::cout << glfwGetTime() - startTime << " Time taken to create first bounce \n";
}

void Application::Update(float deltaTime) {
	camera_->Inputs(window_, deltaTime);
	camera_->updateMatrix(45.0f, 0.1f, 500.0f);
	DebugInputs();
	//averageFPS += 1 / deltaTime;
	///timeSinceStarted += deltaTime;
	//if (timeSinceStarted > 30) {
		//std::cout << averageFPS / timeSinceStarted << " This is the average fps over last 30 seconds \n";
	//}
	//computeShaderTest();
}

void Application::Draw() {
	// ------------------------------------------------------------------- // 
	// --------------------- Draw the scene normally --------------------- //
	// ------------------------------------------------------------------- //
	//CreateVoxels();
	//double timer = glfwGetTime();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Draw to the screen  
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth_, windowHeight_);
	//glViewport(0, 0, voxelTexture_.size, voxelTexture_.size);
	// Set clear color and clear
	glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 viewMatrix = camera_->view;
	glm::mat4 projectionMatrix = camera_->projection;

	glUseProgram(defaultShader->ID);

	glm::vec3 camPos = camera_->Position;
	glUniform3f(glGetUniformLocation(defaultShader->ID, "CameraPosition"), camPos.x, camPos.y, camPos.z);
	glUniform3f(glGetUniformLocation(defaultShader->ID, "LightDirection"), lightDirection_.x, lightDirection_.y, lightDirection_.z);
	glUniform1f(glGetUniformLocation(defaultShader->ID, "VoxelGridWorldSize"), voxelGridWorldSize_);
	glUniform1i(glGetUniformLocation(defaultShader->ID, "VoxelDimensions"), voxelTextureSize);

	glUniform1f(glGetUniformLocation(defaultShader->ID, "ShowDiffuse"), showDirectDiffuse);
	glUniform1f(glGetUniformLocation(defaultShader->ID, "ShowIndirectDiffuse"), showIndirectDiffuse);
	glUniform1f(glGetUniformLocation(defaultShader->ID, "ShowIndirectSpecular"), showIndirectSpecular);
	glUniform1f(glGetUniformLocation(defaultShader->ID, "ShowAmbientOcculision"), showAmbientOcclusion);
	glUniform1f(glGetUniformLocation(defaultShader->ID, "SaveLightToVoxel"), doubleBounce);

	glUniformMatrix4fv(glGetUniformLocation(defaultShader->ID, "ProjX"), 1, GL_FALSE, &projX_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(defaultShader->ID, "ProjY"), 1, GL_FALSE, &projY_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(defaultShader->ID, "ProjZ"), 1, GL_FALSE, &projZ_[0][0]);


	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, depthTexture_.textureID);
	glUniform1i(glGetUniformLocation(defaultShader->ID, "ShadowMap"), 5);



	/// This is for a implementation of multiple light bounces. Should be done in a compute shader instead
	if (doubleBounce) {
		// Bind bounce texture
		glActiveTexture(GL_TEXTURE0 + 7);
		glBindTexture(GL_TEXTURE_3D, voxelTextureBounce_.textureID);
		glUniform1i(glGetUniformLocation(defaultShader->ID, "VoxelTexture"), 7);

	}

	if(!doubleBounce) {
		glActiveTexture(GL_TEXTURE0 + 6);
		glBindTexture(GL_TEXTURE_3D, voxelTexture_.textureID);
		glUniform1i(glGetUniformLocation(defaultShader->ID, "VoxelTexture"), 6);
	}


	if (showMeshRender) {
		model->Draw(*defaultShader, *camera_, depthViewProjectionMatrix_);
		//glActiveTexture(GL_TEXTURE7);
		//glBindTexture(GL_TEXTURE_3D, voxelTextureBounce_.textureID);
		//glGenerateMipmap(GL_TEXTURE_3D);
	}

	if (showVoxels) {
		// Draw voxels for debugging (can't draw large voxel sets like 512^3)
		double currentTime = glfwGetTime();
		drawVoxels();

		//This is the shadow map
		//drawTextureQuad(depthTexture_.textureID);
	}
	if(showShadowMap) {
		showShadowMapDebug(depthTexture_.textureID);
	}

	//std::cout << glfwGetTime() - timer << " Time to cone trace normally \n\n";
}


void Application::computeShaderTest() {

	//GLuint query;
	//glGenQueries(1, &query);

	//glBeginQuery(GL_TIME_ELAPSED, query);

	glm::mat4 viewMatrix = camera_->view;
	glm::mat4 projectionMatrix = camera_->projection;

	//double startTime = glfwGetTime();
	glUseProgram(computeShader->ID);
	//std::cout << glfwGetTime() - startTime << " Time taken swapt to compute shader \n";
	
	//startTime = glfwGetTime();
	//glBindImageTexture(1, voxelTextureBounce_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUniform1i(glGetUniformLocation(computeShader->ID, "bounceTexture"), 1);
	//std::cout << glfwGetTime() - startTime << " Time taken to bind voxelTextureBounce to computeShader \n";
	//glBindImageTexture(1, voxelTexture_.textureID, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
	
	//startTime = glfwGetTime();
	glActiveTexture(GL_TEXTURE0 + 0);
	//glBindTexture(GL_TEXTURE_3D, voxelTexture_.textureID);
	glUniform1i(glGetUniformLocation(computeShader->ID, "VoxelTexture"), 0);
	//std::cout << glfwGetTime() - startTime << " Time taken to send voxelTexture to computeShader \n";

	//startTime = glfwGetTime();
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_3D, voxelNormalTexture_.textureID);
	glUniform1i(glGetUniformLocation(computeShader->ID, "VoxelNormalTexture"), 2);
	//std::cout << glfwGetTime() - startTime << " Time taken to send voxelNormalTexture to computeShader \n";

	glUniform3f(glGetUniformLocation(computeShader->ID, "LightDirection"), lightDirection_.x, lightDirection_.y, lightDirection_.z);
	glUniform1i(glGetUniformLocation(computeShader->ID, "VoxelDimensions"), voxelTextureSize);

	//startTime = glfwGetTime();
	glDispatchCompute(voxelTextureSize, voxelTextureSize, voxelTextureSize);

	// Make sure all dispatches are completed.
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	glActiveTexture(GL_TEXTURE0 + 7);
	//glBindTexture(GL_TEXTURE_3D, voxelTextureBounce_.textureID);
	glGenerateMipmap(GL_TEXTURE_3D);
	//std::cout << glfwGetTime() - startTime << " Time taken for compute shader and mip mapping them \n";

	//glEndQuery(GL_TIME_ELAPSED);
	//GLint64 elapsed_time;
	//glGetQueryObjecti64v(query, GL_QUERY_RESULT, &elapsed_time);
	//std::cout << "Time elapsed: " << elapsed_time << " nanoseconds" << std::endl;

}

bool Application::SetupShadowMap() {
	// Create framebuffer for shadow map
	glGenFramebuffers(1, &depthFramebuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer_);

	// Depth texture
	depthTexture_.width = depthTexture_.height = 4096 * 4;

	viewMatrix = glm::lookAt(lightDirection_, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	projectionMatrix = glm::ortho	<float>(-120, 120, -120, 120, -500, 500);
	depthViewProjectionMatrix_ = projectionMatrix * viewMatrix;

	glGenTextures(1, &depthTexture_.textureID);
	glBindTexture(GL_TEXTURE_2D, depthTexture_.textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, depthTexture_.width, depthTexture_.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture_.textureID, 0);
	// No color output
	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Error creating framebuffer" << std::endl;
		return false;
	}

	return true;
}

void Application::CreateShadowMap() {
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glm::mat4 viewMatrix = glm::lookAt(lightDirection_, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 projectionMatrix = glm::ortho	<float>(-120, 120, -120, 120, -500, 500);
	depthViewProjectionMatrix_ = projectionMatrix * viewMatrix;

	// Draw to depth frame buffer instead of screen
	glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer_);
	// Set viewport of framebuffer size
	glViewport(0, 0, depthTexture_.width, depthTexture_.height);
	// Set clear color and clear
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	model->CreateDepthTexture(*shadowMapShader, *camera_, depthViewProjectionMatrix_);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth_, windowHeight_);
}

bool Application::Initialize3DTextures(Texture3D & Texture, GLint textureCoding, GLint dataType) {
	Texture.size = voxelTextureSize;

	glEnable(GL_TEXTURE_3D);

	glGenTextures(1, &Texture.textureID);
	glBindTexture(GL_TEXTURE_3D, Texture.textureID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Fill 3D texture with empty values

	size_t numVoxels = Texture.size * Texture.size * Texture.size;
	if (dataType == GL_UNSIGNED_BYTE) {
		GLubyte* data = new GLubyte[numVoxels * 4];
		//GLubyte* data = new GLubyte[numVoxels * 4];
		// Initialize voxel data
		std::fill_n(data, numVoxels * 4, 0);

		glTexImage3D(GL_TEXTURE_3D, 0, textureCoding, Texture.size, Texture.size, Texture.size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		delete[] data;
	}
	else if (dataType == GL_FLOAT) {
		GLfloat* data = new GLfloat[numVoxels * 4];
		//GLubyte* data = new GLubyte[numVoxels * 4];
		// Initialize voxel data
		std::fill_n(data, numVoxels * 4, 0);

		glTexImage3D(GL_TEXTURE_3D, 0, textureCoding, Texture.size, Texture.size, Texture.size, 0, GL_RGBA, GL_FLOAT, data);

		delete[] data;
	}
	else {
		std::cerr << "Invalid data type" << std::endl;
		return false;
	}


	glGenerateMipmap(GL_TEXTURE_3D);

	// Create projection matrices used to project stuff onto each axis in the voxelization step
	float size = voxelGridWorldSize_;
	// left, right, bottom, top, zNear, zFar
	projectionMatrix = glm::ortho(-size * 0.5f, size * 0.5f, -size * 0.5f, size * 0.5f, size * 0.5f, size * 1.5f);
	projX_ = projectionMatrix * glm::lookAt(glm::vec3(size, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	projY_ = projectionMatrix * glm::lookAt(glm::vec3(0, size, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
	projZ_ = projectionMatrix * glm::lookAt(glm::vec3(0, 0, size), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	return true;
}

// Basic for directional light
void Application::InitializeLighting(Shader * shader) {
	shader->Activate();
	glUniform3f(glGetUniformLocation(shader->ID, "lightDirection"), lightDirection_.x, lightDirection_.y, lightDirection_.z);
}

void Application::CreateVoxels() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, voxelTexture_.size, voxelTexture_.size);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(voxelInitializaton->ID);

	// Set uniforms
	glUniform1i(glGetUniformLocation(voxelInitializaton->ID, "VoxelDimensions"), voxelTexture_.size);
	glUniformMatrix4fv(glGetUniformLocation(voxelInitializaton->ID, "ProjX"), 1, GL_FALSE, &projX_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelInitializaton->ID, "ProjY"), 1, GL_FALSE, &projY_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelInitializaton->ID, "ProjZ"), 1, GL_FALSE, &projZ_[0][0]);

	// Bind depth texture
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, depthTexture_.textureID);
	glUniform1i(glGetUniformLocation(voxelInitializaton->ID, "ShadowMap"), 5);

	// Bind single level of texture to image unit so we can write to it from shaders
	glActiveTexture(GL_TEXTURE0 + 0);
	//glBindImageTexture(6, voxelTexture_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUniform1i(glGetUniformLocation(voxelInitializaton->ID, "VoxelTexture"), 0);

	// Bind single level of texture to image unit so we can write to it from shaders
	glActiveTexture(GL_TEXTURE0 + 2);
	//glBindImageTexture(0, voxelNormalTexture_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glUniform1i(glGetUniformLocation(voxelInitializaton->ID, "VoxelNormalTexture"), 2);

	model->DrawVoxels(*voxelInitializaton, *camera_, depthViewProjectionMatrix_);

	glActiveTexture(GL_TEXTURE0 + 0);
	//glBindTexture(GL_TEXTURE_3D, voxelTexture_.textureID);
	glGenerateMipmap(GL_TEXTURE_3D);

	// Reset viewport
	glViewport(0, 0, windowWidth_, windowHeight_);
}

void Application::UpdateVoxels() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, voxelTexture_.size, voxelTexture_.size);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(voxelizationShader->ID);

	// Set uniforms
	glUniform1i(glGetUniformLocation(voxelizationShader->ID, "VoxelDimensions"), voxelTexture_.size);
	glUniformMatrix4fv(glGetUniformLocation(voxelizationShader->ID, "ProjX"), 1, GL_FALSE, &projX_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelizationShader->ID, "ProjY"), 1, GL_FALSE, &projY_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelizationShader->ID, "ProjZ"), 1, GL_FALSE, &projZ_[0][0]);

	// Bind depth texture
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, depthTexture_.textureID);
	glUniform1i(glGetUniformLocation(voxelizationShader->ID, "ShadowMap"), 5);

	// Bind single level of texture to image unit so we can write to it from shaders
	glActiveTexture(GL_TEXTURE0 + 0);
	//glBindImageTexture(6, voxelTexture_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUniform1i(glGetUniformLocation(voxelizationShader->ID, "VoxelTexture"), 0);

	// Bind single level of texture to image unit so we can write to it from shaders
	//glActiveTexture(GL_TEXTURE0 + 2);
	//glBindImageTexture(0, voxelNormalTexture_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	//glUniform1i(glGetUniformLocation(voxelizationShader->ID, "VoxelNormalTexture"), 2);

	model->DrawVoxels(*voxelizationShader, *camera_, depthViewProjectionMatrix_);

	glActiveTexture(GL_TEXTURE0 + 0);
	//glBindTexture(GL_TEXTURE_3D, voxelTexture_.textureID);
	glGenerateMipmap(GL_TEXTURE_3D);

	// Reset viewport
	glViewport(0, 0, windowWidth_, windowHeight_);
}

// For debugging
void Application::drawVoxels() {
	glUseProgram(visualizeVoxelsShader->ID);

	size_t numVoxels = voxelTexture_.size * voxelTexture_.size * voxelTexture_.size;
	float voxelSize = voxelGridWorldSize_ / voxelTexture_.size;
	glUniform1i(glGetUniformLocation(visualizeVoxelsShader->ID, "Dimensions"), voxelTexture_.size);
	glUniform1i(glGetUniformLocation(visualizeVoxelsShader->ID, "TotalNumVoxels"), numVoxels);
	glUniform1f(glGetUniformLocation(visualizeVoxelsShader->ID, "VoxelSize"), voxelSize);
	glm::mat4 modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(voxelSize)), glm::vec3(0, 0, 0));
	glm::mat4 viewMatrix = camera_->view;
	glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
	glm::mat4 projectionMatrix = camera_->projection;
	glUniformMatrix4fv(glGetUniformLocation(visualizeVoxelsShader->ID, "ModelViewMatrix"), 1, GL_FALSE, &modelViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(visualizeVoxelsShader->ID, "ProjectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);


	if (doubleBounce) {
		glActiveTexture(GL_TEXTURE0 + 7);
		glBindTexture(GL_TEXTURE_3D, voxelTextureBounce_.textureID);
		glUniform1i(glGetUniformLocation(visualizeVoxelsShader->ID, "VoxelsTexture"), 7);
	}
	if (!doubleBounce) {
		glActiveTexture(GL_TEXTURE0 + 6);
		glBindTexture(GL_TEXTURE_3D, voxelTexture_.textureID);
		glUniform1i(glGetUniformLocation(visualizeVoxelsShader->ID, "VoxelsTexture"), 6);
	}


	glBindVertexArray(visualizeVoxelsShader->ID);
	glDrawArrays(GL_POINTS, 0, numVoxels);

	glBindVertexArray(0);
	glUseProgram(0);
}

void Application::DebugInputs() {
	newShadowMapNeeded = false;
	if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS) {
		showVoxels = true;
		std::cout << "Voxel visualization on\n";
	}
	if (glfwGetKey(window_, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
		showVoxels = false;
		std::cout << "Voxel visualization off\n";
	}
	if (glfwGetKey(window_, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
		showMeshRender = false;
		std::cout << "Mesh render off\n";
	}
	if (glfwGetKey(window_, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
		showMeshRender = true;
		std::cout << "Mesh render on\n";
	}

	if (!press1_ && glfwGetKey(window_, GLFW_KEY_1) == GLFW_PRESS) {
		showDirectDiffuse = !showDirectDiffuse;
		press1_ = true;
	}
	if (!press2_ && glfwGetKey(window_, GLFW_KEY_2) == GLFW_PRESS) {
		showIndirectDiffuse = !showIndirectDiffuse;
		press2_ = true;
	}
	if (!press3_ && glfwGetKey(window_, GLFW_KEY_3) == GLFW_PRESS) {
		showIndirectSpecular = !showIndirectSpecular;
		press3_ = true;
	}
	if (!press4_ && glfwGetKey(window_, GLFW_KEY_4) == GLFW_PRESS) {
		showAmbientOcclusion = !showAmbientOcclusion;
		press4_ = true;
	}
	if (glfwGetKey(window_, GLFW_KEY_0) == GLFW_PRESS) {
		doubleBounce = false;
		newShadowMapNeeded = true;
	}
	if (glfwGetKey(window_, GLFW_KEY_9) == GLFW_PRESS) {
		doubleBounce = true;
		newShadowMapNeeded = true;
	}

	if (glfwGetKey(window_, GLFW_KEY_1) == GLFW_RELEASE) {
		press1_ = false;
	}
	if (glfwGetKey(window_, GLFW_KEY_2) == GLFW_RELEASE) {
		press2_ = false;
	}
	if (glfwGetKey(window_, GLFW_KEY_3) == GLFW_RELEASE) {
		press3_ = false;
	}
	if (glfwGetKey(window_, GLFW_KEY_4) == GLFW_RELEASE) {
		press4_ = false;
	}
	if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS) {
		lightDirection_.z += 0.1;
		newShadowMapNeeded = true;
	}
	if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS) {
		lightDirection_.z -= 0.1;
		newShadowMapNeeded = true;
	}
	if (glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS) {
		lightDirection_.x -= 0.1;
		newShadowMapNeeded = true;
	}
	if (glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		lightDirection_.x += 0.1;
		newShadowMapNeeded = true;
	}
	if (glfwGetKey(window_, GLFW_KEY_PERIOD) == GLFW_PRESS) {
		showShadowMap = true;
	}
	if (glfwGetKey(window_, GLFW_KEY_COMMA) == GLFW_PRESS) {
		showShadowMap = false;
	}
	if (newShadowMapNeeded) {
		modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(1)), glm::vec3(0.0f));
		depthModelViewProjectionMatrix = depthViewProjectionMatrix_ * modelMatrix;


		//double startTime = glfwGetTime();
		CreateShadowMap();
		//std::cout << glfwGetTime() - startTime << "Time taken to create shadowmap \n";

		//startTime = glfwGetTime();
		UpdateVoxels();

		//std::cout << glfwGetTime() - startTime << "Time taken to voxelize scene \n";
		//CreateAdditionalBounces();
		//lightInjection();
		//startTime = glfwGetTime();
		if (doubleBounce == true) {
			computeShaderTest();
		}
		//std::cout << glfwGetTime() - startTime << "Time taken to compute shader scene \n";
		//std::cout << "New light direction: " << lightDirection_.x << " " << lightDirection_.y << " " << lightDirection_.z << "\n";
		std::cout << camera_->Orientation.x << " " << camera_->Orientation.y << " " << camera_->Orientation.z << " camera orientation \n";
		std::cout << camera_->Position.x << " " << camera_->Position.y << " " << camera_->Position.z << " camera position \n";
		newShadowMapNeeded = false;
	}

}

// For debugging
void Application::showShadowMapDebug(GLuint textureID) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 300, 300);

	glUseProgram(shadowMapDebugShader->ID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(glGetUniformLocation(shadowMapDebugShader->ID, "Texture"), 0);

	glBindVertexArray(quadVertexArray_);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
}