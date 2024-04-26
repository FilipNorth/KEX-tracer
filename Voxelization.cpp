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
    Camera& camera
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

    voxelTexture->BindAsImage(0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(0, voxelTexture->textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);



    // Bind the voxelization shader and pass necessary uniforms
    glm::mat4 modelMatrix = glm::mat4(0.00800000037997961);
    //glm::mat4 modelMatrix = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(voxelShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniform3i(glGetUniformLocation(voxelShader.ID, "gridSize"), 64, 64, 64);

    // Take care of the camera Matrix
    glUniform3f(glGetUniformLocation(voxelShader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    camera.Matrix(voxelShader, "camMatrix");

    // Ensure no textures interfere with the process
    //glBindTexture(GL_TEXTURE_2D, 0);

    // Setup the viewport to match the grid size if needed
    //glViewport(0, 0, 64, 64);

    // Draw the mesh as points or whatever form is required for voxelization
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

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

