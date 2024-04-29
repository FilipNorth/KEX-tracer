#include"Model.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include "Voxelizer.h"


const unsigned int width = 1920;
const unsigned int height = 1080;

//lightCube, reveals lightPos more easily
GLfloat lightVertices[] =
{ //     COORDINATES     // (local coords, are moved by lightMatrix to global in lightVertShader)
	-0.1f, -0.1f,  0.1f,
	-0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f,  0.1f,
	-0.1f,  0.1f,  0.1f,
	-0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f,  0.1f
};

GLuint lightIndices[] =
{
	0, 1, 2,
	0, 2, 3,
	0, 4, 7,
	0, 7, 3,
	3, 7, 6,
	3, 6, 2,
	2, 6, 5,
	2, 5, 1,
	1, 5, 4,
	1, 4, 0,
	4, 5, 6,
	4, 6, 7
};

GLfloat QuadVert[]{
	-1. , -1. , 0.0,
	1. , -1. , 0.0,
	1. , 1. , 0.0,
	-1. , 1. , 0.0
};

GLuint QuadInd[]{
	0, 1, 2,
	0, 2, 3
};

int main()
{
	//main1();
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "KEX-tracer", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, 64, 64);

	// Generates Shader object using shaders default.vert and default.frag
	Shader defaultShader("Shaders/default.vert", "Shaders/default.geom", "Shaders/default.frag");
	Shader voxelShader("Shaders/voxel.vert", "Shaders/voxel.frag");
	Shader normalShader("Shaders/default.vert", "Shaders/normal.geom", "Shaders/normal.frag");

	Shader testingShaders("Shaders/voxelTesting.vert", "Shaders/voxelTesting.geom", "Shaders/voxelTesting.frag");
	GLenum gl_error = glGetError();
	if (gl_error != GL_NO_ERROR) {
		std::cout << "OpenGL Error: " << gl_error << std::endl;
	}

	// Shader for light cube
	Shader lightShader("Shaders/light.vert", "Shaders/light.frag");
	// Generates Vertex Array Object and binds it
	VAO lightVAO;

	VAO quadVAO;
	quadVAO.Bind();
	VBO quadVBO(QuadVert, sizeof(QuadVert));
	EBO quadEBO(QuadInd, sizeof(QuadInd));

	quadVAO.LinkAttrib(quadVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	quadVAO.Unbind();

	lightVAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO lightVBO(lightVertices, sizeof(lightVertices));
	// Generates Element Buffer Object and links it to indices
	EBO lightEBO(lightIndices, sizeof(lightIndices));
	// Links VBO attributes such as coordinates and colors to VAO
	lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	// Unbind all to prevent accidentally modifying them
	lightVAO.Unbind();
	lightVBO.Unbind();
	lightEBO.Unbind();

	// Take care of all the light related things
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 voxelLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(2.5f, 0.8f, 4.0f);
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, lightPos);

	defaultShader.Activate();
	glUniform4f(glGetUniformLocation(defaultShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(defaultShader.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	voxelShader.Activate();
	glUniform4f(glGetUniformLocation(voxelShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(voxelShader.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	lightShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
	glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);

	testingShaders.Activate();
	glUniform4f(glGetUniformLocation(testingShaders.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(testingShaders.ID, "lightColor2"), voxelLightColor.x, voxelLightColor.y, voxelLightColor.z);
	glUniform3f(glGetUniformLocation(testingShaders.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	gl_error = glGetError();
	if (gl_error != GL_NO_ERROR) {
		std::cout << "OpenGL Error: " << gl_error << std::endl;
	}

	GLint val = GL_FALSE;
	glGetShaderiv(testingShaders.ID, GL_COMPILE_STATUS, &val);
	if (val != GL_TRUE)
	{
		std::cout << "Compilation failed in main" << std::endl;
		// compilation failed
	}

	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Creates camera object
	Camera camera(width, height, glm::vec3(30.0f, 45.0f, 12.0f));

	// Load in a model
	Model model("Models/Sponza-glTF/Sponza.gltf");
	//("Models/Stanford_Bunny/scene.gltf");

	// Original code from the tutorial
	// Model model("models/bunny/scene.gltf");
	//model.DrawVoxels(voxelShader, camera);

	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, 64, 64);
		// Specify the color of the background
		glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Handles camera inputs
		camera.Inputs(window);
		// Updates and exports the camera matrix to the Vertex Shader
		camera.updateMatrix(45.0f, 0.1f, 100.0f);
		// Draw a model
		model.DrawVoxels(testingShaders, camera);
		model.VisualizeVoxels(voxelShader, camera);
		//model.Draw(defaultShader, camera);
		//model.Draw(normalShader, camera);

		// Tells OpenGL which Shader Program we want to use
		//lightShader.Activate();
		// Export the camMatrix to the Vertex Shader of the light cube
		//camera.Matrix(lightShader, "camMatrix");
		// Bind the VAO so OpenGL knows to use it
		//lightVAO.Bind();
		// Draw primitives, number of indices, datatype of indices, index of indices
		//glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(int), GL_UNSIGNED_INT, 0);

		voxelShader.Activate();

		quadVAO.Bind();

		glViewport(0, 0, width, height);

		glDrawElements(GL_TRIANGLES, sizeof(QuadInd) / sizeof(int), GL_UNSIGNED_INT, 0);

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}


	// Delete all the objects we've created
	defaultShader.Delete();
	voxelShader.Delete();
	lightShader.Delete();
	testingShaders.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}