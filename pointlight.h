#pragma once

#include <glad/glad.h>

#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/// <summary> A simple point light. </summary>
class PointLight {
public:
	bool tweakable = true;
	glm::vec3 position, color;
	PointLight(glm::vec3 _position = { 0, 0, 0 }, glm::vec3 _color = { 1, 1, 1 }) : position(_position), color(_color) {}
	void Upload(GLuint program, GLuint index) const {
		glUniform3fv(glGetUniformLocation(program, ("pointLights[" + std::to_string(index) + "].position").c_str()), 1, glm::value_ptr(position));
		glUniform3fv(glGetUniformLocation(program, ("pointLights[" + std::to_string(index) + "].color").c_str()), 1, glm::value_ptr(color));
	}
};