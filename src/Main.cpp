#include"Model.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include "Application.h"

const unsigned int width = 1920;
const unsigned int height = 1080;
float timer = 0.0;

int main()
{
	//main1();
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwSwapInterval(0);


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

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
	}


	GLint numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	for (int i = 0; i < numExtensions; i++) {
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		std::cout << i + 1 << ": " << extension << std::endl;
	}

	Application app(height, width, window);
	app.Initialize();
	double testTime = 0;
	double averageFPS = 0;
	double previousTime, currentTime;
	previousTime = glfwGetTime();
	testTime = previousTime;
	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		// Update timer
		currentTime = glfwGetTime();
		float deltaTime = float(currentTime - previousTime);
		previousTime = currentTime;

		if (timer > 2.0) {
			std::cout << "FPS: " << 1.0 / deltaTime << std::endl;
			averageFPS += 1 / deltaTime;
			timer = 0.0;
		}
		else {
			timer += deltaTime;
		}
		if (currentTime - testTime > 30) {
			averageFPS / (currentTime - testTime);
			//std::cout << averageFPS << " This is the average FPS over the last 30 seconds\n";
		}
		// Specify the color of the background
		glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT);


		app.Update(deltaTime);

		app.Draw();

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}

	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}