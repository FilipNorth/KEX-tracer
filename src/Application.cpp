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
	storeImageDataCompute = new Shader("../../../../Shaders/StoreTextureData.comp");
	
	// Load model
	std::cout << "Loading model" << "\n";

	double startTime = glfwGetTime();
	model = new Model("../../../../Models/Sponza-glTF/Sponza.gltf");
	std::cout <<  glfwGetTime() - startTime << " Time taken to load model \n";
	std::cout << "Model loaded" << "\n";

	/// Initialize Shadow Map ///
	SetupShadowMap();

	//glEnable(GL_ARB_sparse_texture);
	glEnable(GL_ARB_shader_storage_buffer_object);
	glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glEnable(GL_ARB_shader_image_load_store);

	/// Initialize 3D Textures ///
	Initialize3DTextures(voxelTexture_, GL_RGBA8, GL_UNSIGNED_BYTE);
	Initialize3DTextures(voxelTextureBounce_, GL_RGBA8, GL_UNSIGNED_BYTE);
	Initialize3DTextures(voxelNormalTexture_, GL_RGBA16F, GL_FLOAT);
	//glDisable(GL_ARB_sparse_texture);

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

	UpdateVoxels();

	Draw();

	//sparseTextureCommitment(voxelTexture_.textureID);
	//sparseTextureCommitment(voxelNormalTexture_.textureID);
	//sparseTextureCommitment(voxelTextureBounce_.textureID);

	CreateVoxels();

	UpdateVoxels();

	startTime = glfwGetTime();
	computeShaderTest();

	//glGenerateMipmap(GL_TEXTURE_3D);
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

	//glEnable(GL_ARB_sparse_texture);
	glEnable(GL_ARB_shader_image_load_store);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//double startTime = glfwGetTime();
	glUseProgram(computeShader->ID);

	// Bind bounce texture as image for writing
	glBindImageTexture(1, voxelTextureBounce_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

	// Bind voxel texture for reading
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, voxelTexture_.textureID);
	glUniform1i(glGetUniformLocation(computeShader->ID, "VoxelTexture"), 0);

	// Bind normal texture for reading
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, voxelNormalTexture_.textureID);
	glUniform1i(glGetUniformLocation(computeShader->ID, "VoxelNormalTexture"), 1);
	//std::cout << glfwGetTime() - startTime << " Time taken to send voxelNormalTexture to computeShader \n";

	glUniform3f(glGetUniformLocation(computeShader->ID, "LightDirection"), lightDirection_.x, lightDirection_.y, lightDirection_.z);
	glUniform1i(glGetUniformLocation(computeShader->ID, "VoxelDimensions"), voxelTextureSize);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PageUsageInfo), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo); // Binding to binding point 0
	//startTime = glfwGetTime();
	
	//for (int i = 0; i < sortedDispatchX.size(); i++) {
	//	for(int j = 0; j < sortedDispatchY.size(); j++){
	//		for(int k = 0; k < sortedDispatchZ.size(); k++){
	//				//std::cout << "Dispatching " << i << " " << j << " " << k << "\n";
	//			glm::ivec3 offset = glm::ivec3(sortedDispatchX[i], sortedDispatchY[j], sortedDispatchZ[k]);
	//
	//			glUniform3f(glGetUniformLocation(computeShader->ID, "DispatchOffset"), offset.x * 32, offset.y * 32, offset.z * 32);
	//			glDispatchCompute(32, 32, 32);
	//
	//		}
	//	}
	//
	//}

	///for(int i = 0; i < dispatchInfoList.size(); i++){
	//	glUniform3f(glGetUniformLocation(computeShader->ID, "DispatchOffset"), dispatchInfoList[i].x * 32, dispatchInfoList[i].y * 32, dispatchInfoList[i].z * 32);
	//	glDispatchCompute(32, 32, 32);
	//}
	
	glDispatchCompute(voxelTextureSize, voxelTextureSize, voxelTextureSize);

	// Make sure all dispatches are completed.
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	//glActiveTexture(GL_TEXTURE0 + 7);
	//glBindTexture(GL_TEXTURE_3D, voxelTextureBounce_.textureID);

	VoxelMipMapper(voxelTextureBounce_, "bounceTexture");
	//glGenerateMipmap(GL_TEXTURE_3D);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
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

	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_SPARSE_ARB, GL_TRUE);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 4);  // Level 5 is your minimum
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, mipLevels);  // Preventing generation of mipmaps below 16x16x16


	glTexStorage3D(GL_TEXTURE_3D, mipLevels, textureCoding, Texture.size, Texture.size, Texture.size);

	//glTexPageCommitmentARB(GL_TEXTURE_3D, 0, xoffset, yoffset, zoffset, Texture.size, Texture.size, Texture.size, GL_TRUE);
	/*
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

	*/

	//glGenerateMipmap(GL_TEXTURE_3D);

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

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PageUsageInfo), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo); // Binding to binding point 0

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

	//sparseTextureCommitment();

	glActiveTexture(GL_TEXTURE0 + 0);
	//glBindTexture(GL_TEXTURE_3D, voxelTexture_.textureID);
	//glGenerateMipmap(GL_TEXTURE_3D);

	// Reset viewport
	glViewport(0, 0, windowWidth_, windowHeight_);
}

void Application::UpdateVoxels() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, voxelTexture_.size, voxelTexture_.size);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glEnable(GL_ARB_shader_image_load_store);
	glEnable(GL_ARB_shader_storage_buffer_object);

	glUseProgram(voxelizationShader->ID);

	// Uniform setup
	glUniform1i(glGetUniformLocation(voxelizationShader->ID, "VoxelDimensions"), voxelTexture_.size);
	glUniformMatrix4fv(glGetUniformLocation(voxelizationShader->ID, "ProjX"), 1, GL_FALSE, &projX_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelizationShader->ID, "ProjY"), 1, GL_FALSE, &projY_[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(voxelizationShader->ID, "ProjZ"), 1, GL_FALSE, &projZ_[0][0]);

	// Bind depth texture
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, depthTexture_.textureID);
	glUniform1i(glGetUniformLocation(voxelizationShader->ID, "ShadowMap"), 5);

	// Bind voxel texture as image for writing
	glBindImageTexture(0, voxelTexture_.textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUniform1i(glGetUniformLocation(voxelizationShader->ID, "VoxelTexture"), 0);

	model->DrawVoxels(*voxelizationShader, *camera_, depthViewProjectionMatrix_);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// Reset viewport
	glViewport(0, 0, windowWidth_, windowHeight_);

	glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
	VoxelMipMapper(voxelTexture_, "VoxelTexture");
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

		//double startTime = glfwGetTime();
		UpdateVoxels();

		//std::cout << glfwGetTime() - startTime << "Time taken to voxelize scene \n";
		//CreateAdditionalBounces();
		//lightInjection();
		//startTime = glfwGetTime();
		if (doubleBounce == true) {
			//startTime = glfwGetTime();
			computeShaderTest();
			//std::cout << glfwGetTime() - startTime << "Time taken to compute shader scene" << "\n";
		}
		//std::cout << glfwGetTime() - startTime << "Time taken to compute shader scene \n";
		//std::cout << "New light direction: " << lightDirection_.x << " " << lightDirection_.y << " " << lightDirection_.z << "\n";
		//std::cout << camera_->Orientation.x << " " << camera_->Orientation.y << " " << camera_->Orientation.z << " camera orientation \n";
		//std::cout << camera_->Position.x << " " << camera_->Position.y << " " << camera_->Position.z << " camera position \n";
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

void Application::sparseTextureCommitment(GLuint textureID) {
	glBindTexture(GL_TEXTURE_3D, textureID);

	PageUsageInfo* info = (PageUsageInfo*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	if (!info) {
		std::cerr << "Failed to map buffer." << std::endl;
		return;
	}

	for (int i = 0; i < voxelTexture_.size * 8; i++) {
		if (info->baseLevel[i] == 1) {
			info->mipLevel1[i / (8)] = 1;
			info->mipLevel2[i / (8*8)] = 1;
			info->mipLevel3[i / (8*8*8)] = 1;
			info->mipLevel4[i / (8*8*8*8)] = 1;
		}

	}

	dispatchX.resize(voxelTexture_.size / pageSize);
	dispatchY.resize(voxelTexture_.size / pageSize);
	dispatchZ.resize(voxelTexture_.size / pageSize);
	// Commit pages for the base level
	int baseDimension = voxelTexture_.size;
	commitPagesForLevel(0, baseDimension, info->baseLevel);

	// Automatically determine the number of mip levels (assuming mip levels halve the dimensions each step)
	int level = 1;
	int currentDimension = baseDimension / 2;

	while (currentDimension >= pageSize) {  // Continue until the dimension is too small
		int pagesPerDimension = currentDimension / pageSize;
		int totalPages = pagesPerDimension * pagesPerDimension * pagesPerDimension;
		int* currentLevelFlags = nullptr;

		switch (level) {
		case 1: currentLevelFlags = info->mipLevel1; break;
		case 2: currentLevelFlags = info->mipLevel2; break;
		case 3: currentLevelFlags = info->mipLevel3; break;
		case 4: currentLevelFlags = info->mipLevel4; break;
		default: break;  // Add more cases if there are more mip levels
		}

		if (currentLevelFlags) {
			commitPagesForLevel(level, currentDimension, currentLevelFlags);
		}

		currentDimension /= 2;
		level++;
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	for(int i = 0; i < dispatchX.size(); i++) {
		if(dispatchX[i] == 1) {
			dispatchSizeX++;
			sortedDispatchX.push_back(i);
		}
		if(dispatchY[i] == 1) {
			dispatchSizeY++;
			sortedDispatchY.push_back(i);
		}
		if(dispatchZ[i] == 1) {
			dispatchSizeZ++;
			sortedDispatchZ.push_back(i);
		}

	}
	for (int i = 0; i < dispatchSizeX; i++) {
		for (int j = 0; j < dispatchSizeY; j++) {
			for (int k = 0; k < dispatchSizeZ; k++) {
				if (dispatchX[sortedDispatchX[i]] == 1 && dispatchY[sortedDispatchY[j]] == 1 && dispatchZ[sortedDispatchZ[k]] == 1) {
					dispatchInfo info = { sortedDispatchX[i], sortedDispatchY[j], sortedDispatchZ[k] };
					dispatchInfoList.push_back(info);
				}
			}
		}
	}

}

void Application::commitPagesForLevel(int mipLevel, int dimension, int* levelArray) {
	int pagesPerDimension = dimension / pageSize;
	int totalPages = pagesPerDimension * pagesPerDimension * pagesPerDimension;
	for (int i = 0; i < totalPages; i++) {
		if (levelArray[i] == 0 && mipLevel == 0) {
			continue;
		}
		int z = i / (pagesPerDimension * pagesPerDimension);
		int y = (i / pagesPerDimension) % pagesPerDimension;
		int x = i % pagesPerDimension;
		glTexPageCommitmentARB(GL_TEXTURE_3D, mipLevel, x * pageSize, y * pageSize, z * pageSize, pageSize, pageSize, pageSize, GL_TRUE);

		if(mipLevel == 0){
			dispatchX[x] = 1;
			dispatchY[y] = 1;
			dispatchZ[z] = 1;
		}

	}
}

void Application::VoxelMipMapper(Texture3D Texture, const GLchar* textureName) {

	//glEnable(GL_ARB_sparse_texture);
	glEnable(GL_ARB_shader_image_load_store);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//double startTime = glfwGetTime();
	glUseProgram(storeImageDataCompute->ID);
	//std::cout << glfwGetTime() - startTime << " Time taken swapt to compute shader \n";

	glUniform1i(glGetUniformLocation(storeImageDataCompute->ID, "VoxelDimensions"), voxelTexture_.size);
	glUniform1i(glGetUniformLocation(storeImageDataCompute->ID, "MipLevels"), mipLevels);

	for (int level = 0; level < mipLevels; level++) {
		std::string uniformName = std::string("mipTextures") + std::to_string(level);
		int uniformLoc = glGetUniformLocation(storeImageDataCompute->ID, uniformName.c_str());
		glBindImageTexture(
			level,            // Use the level as the binding unit for simplicity
			Texture.textureID,
			level,            // The mipmap level to bind
			GL_TRUE,          // Layered targeting the whole 3D texture
			0,                // No layer
			GL_WRITE_ONLY,    // Write only access
			GL_RGBA8
		);// Texture format
		glUniform1i(uniformLoc, level);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PageUsageInfo), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo); // Binding to binding point 0

	for (int level = 1; level <= mipLevels; level++) {
		int levelSize = int(voxelTextureSize) >> level;
		glUniform1i(glGetUniformLocation(storeImageDataCompute->ID, "currentLevel"), level - 1);
		glDispatchCompute(levelSize, levelSize, levelSize);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

}