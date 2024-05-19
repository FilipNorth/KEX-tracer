#pragma once
#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

std::string get_file_contents(const char* filename);

class Shader
{
public:
	// Reference ID of the Shader Program
	GLuint ID;
	const GLuint program = 0;
	// Constructor that build the Shader Program from 2 different shaders
	Shader(const char* vertexFile, const char* geometryFile, const char* fragmentFile);
	Shader(const char* vertexFile, const char* fragmentFile);
	Shader(const char* computeFile);

	// Activates the Shader Program
	void Activate();
	// Deletes the Shader Program
	void Delete();

private:
	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type);
	
};

