#include "Voxelization.h"

Voxelization::Voxelization(std::vector <Vertex>& vertices,
    std::vector <GLuint>& indices,
    std::vector <Texture>& textures, Texture3D* voxelTexture)
{
    Voxelization::vertices = vertices;
    Voxelization::indices = indices;
    Voxelization::textures = textures;
    Voxelization::voxelTexture = *voxelTexture;

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

void Voxelization::Draw
(
    Shader& voxelShader, // This is the shader that will handle the voxelization
    Texture3D* voxelTexture, // ID of the 3D texture to store voxel data
    Camera& camera, 
    glm::mat4 modelMatrix
)
{

    voxelShader.Activate();

    VAO.Bind();


    for (unsigned int i = 0; i < textures.size(); i++)
    {
        std::string num;
        std::string type = textures[i].type;

        textures[i].texUnit(voxelShader, (type + "0").c_str(), textures[i].unit);
        textures[i].Bind();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Settings.

    //glViewport(0, 0, voxelTextureSize, voxelTextureSize);
    //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    //glDisable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);
    //glDisable(GL_BLEND);

    voxelTexture->BindAsImage(0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    //glBindImageTexture(0, voxelTexture->textureID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glm::vec3 proj = glm::vec3(2048, 2048, 2048);
    glm::mat4 voxel_projection_ = glm::ortho(-proj.x, proj.x,
        -proj.y, proj.y,
        proj.z, -proj.z);

    glm::mat4 m = glm::mat4(1);

    m[2][2] = -1.0f;

    // Bind the voxelization shader and pass necessary uniforms
    glm::vec3 position = { 0,0,0 }, scale = { 1,1,1 }, rotation = { 0,0,0 };
    glm::mat4 transform = glm::translate(position) * glm::mat4_cast(glm::quat(rotation)) * glm::scale(scale);

    glm::mat4 model = glm::translate(modelMatrix, camera.Position);
    //glm::mat4 modelMatrix = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(voxelShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(voxel_projection_));
    glUniform3i(glGetUniformLocation(voxelShader.ID, "gridSize"), voxelTexture->width, voxelTexture->height, voxelTexture->depth);

    // Take care of the camera Matrix
    glUniform3f(glGetUniformLocation(voxelShader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    camera.Matrix(voxelShader, "camMatrix");

    // Initialize matrices
    glm::mat4 trans = glm::mat4(1.0f);
    glm::mat4 rot = glm::mat4(1.0f);
    glm::mat4 sca = glm::mat4(1.0f);

    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat rotation2 = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    // Transform the matrices to their correct form
    trans = glm::translate(trans, translation);
    rot = glm::mat4_cast(rotation2);
    sca = glm::scale(sca, scale);

    // Push the matrices to the vertex shader
    glUniformMatrix4fv(glGetUniformLocation(voxelShader.ID, "translation"), 1, GL_FALSE, glm::value_ptr(trans));
    glUniformMatrix4fv(glGetUniformLocation(voxelShader.ID, "rotation"), 1, GL_FALSE, glm::value_ptr(rot));
    glUniformMatrix4fv(glGetUniformLocation(voxelShader.ID, "scale"), 1, GL_FALSE, glm::value_ptr(sca));

    // Ensure no textures interfere with the process
    //glBindTexture(GL_TEXTURE_2D, 0);

    // Setup the viewport to match the grid size if needed
    //glViewport(0, 0, 64, 64);

    // Draw the mesh as points or whatever form is required for voxelization
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear before drawing the quad

    // Restore OpenGL settings
    //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    //glEnable(GL_BLEND);
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    GLenum gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
        std::cout << "OpenGL Error: " << gl_error << std::endl;
    }
}

void Voxelization::visualizeVoxels(
    Shader& voxelShader, // This is the shader that will handle the voxelization
    Texture3D* voxelTexture, // ID of the 3D texture to store voxel data
    Camera& camera
)
{
    voxelShader.Activate();

    VAO.Bind();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Settings.

    //glViewport(0, 0, voxelTextureSize, voxelTextureSize);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    voxelTexture->BindAsImage(0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glBindImageTexture(0, voxelTexture->textureID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glm::vec3 proj = glm::vec3(2048, 2048, 2048);
    glm::mat4 voxel_projection_ = glm::ortho(-proj.x, proj.x,
        -proj.y, proj.y,
        proj.z, -proj.z);

    // Bind the voxelization shader and pass necessary uniforms
    glm::mat4 modelMatrix = glm::mat4(0.00800000037997961);
    //glm::mat4 modelMatrix = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(voxelShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(voxel_projection_));
    glUniform3i(glGetUniformLocation(voxelShader.ID, "gridSize"), voxelTexture->width, voxelTexture->height, voxelTexture->depth);

    // Take care of the camera Matrix
    glUniform3f(glGetUniformLocation(voxelShader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    camera.Matrix(voxelShader, "camMatrix");

    glm::vec3 cameraPos = camera.Position;
    //std::cout << "Camera Position: " << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << std::endl;
    glm::vec3 cameraTarget = camera.Position + camera.Orientation;
    glm::vec3 upVector = camera.Up;

    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, upVector);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1920 / (float)1080, 0.1f, 100.0f);

    glm::mat4 viewProj = projection * view;

    glm::mat4 invViewProj = glm::inverse(viewProj);

    glUniformMatrix4fv(glGetUniformLocation(voxelShader.ID, "invViewProj"), 1, GL_FALSE, glm::value_ptr(invViewProj));

    // Ensure no textures interfere with the process
    //glBindTexture(GL_TEXTURE_2D, 0);

    // Setup the viewport to match the grid size if needed
    //glViewport(0, 0, 64, 64);

    // Draw the mesh as points or whatever form is required for voxelization
    //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // Restore OpenGL settings
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    GLenum gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
        std::cout << "OpenGL Error: " << gl_error << std::endl;
    }
}
