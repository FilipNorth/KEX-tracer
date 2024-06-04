#include "Mesh.h"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures, float scale)
{	
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;
	Mesh::scale = scale;

	VAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO(vertices);
	// Generates Element Buffer Object and links it to indices
	EBO EBO(indices);
	// Links VBO attributes such as coordinates and colors to VAO
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

void Mesh::bindMaterials(Shader& shader) {
	//glBindTexture(GL_TEXTURE_2D, 0);
	shader.Activate();
	VAO.Bind();
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string num;
		std::string type = textures[i].type;

		textures[i].texUnit(shader, (type).c_str(), textures[i].unit);
		textures[i].Bind();
	}

}

void Mesh::Draw
(
	Shader& shader,
	Camera& camera,
	glm::mat4 matrix,
	glm::vec3 translation,
	glm::quat rotation,
	glm::vec3 scale
)
{
	// Bind shader to be able to access uniforms

	//glBindTexture(GL_TEXTURE_2D, 0);


	//shader.Activate();
	//VAO.Bind();

	// Take care of the camera Matrix
	///glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	//camera.Matrix(shader, "camMatrix");

	/*
	// Initialize matrices
	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 rot = glm::mat4(1.0f);
	glm::mat4 sca = glm::mat4(1.0f);

	// Transform the matrices to their correct form
	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	// Push the matrices to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "translation"), 1, GL_FALSE, glm::value_ptr(trans));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "rotation"), 1, GL_FALSE, glm::value_ptr(rot));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "scale"), 1, GL_FALSE, glm::value_ptr(sca));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(matrix));

	*/
	// Draw the actual mesh
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::createShadowMap(Shader& shader, Camera& camera, glm::mat4& depthViewProjectionMatrix) {
	glUseProgram(shader.ID);

	glm::mat4 modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale)), glm::vec3(0.0f)); // Should it be 0?
	glm::mat4 modelViewProjectionMatrix = depthViewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "ModelViewProjectionMatrix"), 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);

	shader.Activate();
	VAO.Bind();

	Draw(shader, camera, modelMatrix); // Which one to use?
}

void Mesh::createVoxels(Shader& shader, Camera& camera, glm::mat4& depthViewProjectionMatrix) {
	bindMaterials(shader);

	// Matrix to transform to light position
	glm::mat4 modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale)), glm::vec3(0.0f));
	glm::mat4 depthModelViewProjectionMatrix = depthViewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "DepthModelViewProjectionMatrix"), 1, GL_FALSE, &depthModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "ModelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);

	Draw(shader, camera, modelMatrix);
}

void Mesh::standardDraw(Shader& shader, Camera& camera, glm::mat4& depthViewProjectionMatrix){
	bindMaterials(shader);

	glm::mat4 modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale)), glm::vec3(0.0f));
	glm::mat4 modelViewMatrix = camera.view * modelMatrix;
	glm::mat4 depthModelViewProjectionMatrix = depthViewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "ViewMatrix"), 1, GL_FALSE, &camera.view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "ModelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "ModelViewMatrix"), 1, GL_FALSE, &modelViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "ProjectionMatrix"), 1, GL_FALSE, &camera.projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "DepthModelViewProjectionMatrix"), 1, GL_FALSE, &depthModelViewProjectionMatrix[0][0]);

	Draw(shader, camera, modelMatrix);

}