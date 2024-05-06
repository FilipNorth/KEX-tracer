#include "Voxel_Cone_Tracing.h"


Voxel_Cone_Tracing::Voxel_Cone_Tracing()
{
}


void Voxel_Cone_Tracing::visualizeVoxels(
    Shader& voxelShader, // This is the shader that will handle the voxelization
    Texture3D* voxelTexture, // ID of the 3D texture to store voxel data
    Camera& camera
)
{
    voxelShader.Activate();

    //VAO.Bind();

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

    glm::vec3 viewDirection = glm::normalize(cameraTarget - cameraPos);

    glUniformMatrix4fv(glGetUniformLocation(voxelShader.ID, "invViewProj"), 1, GL_FALSE, glm::value_ptr(invViewProj));
    glUniform3f(glGetUniformLocation(voxelShader.ID, "viewDirection"), viewDirection.x, viewDirection.y, viewDirection.z);

    glUniform3f(glGetUniformLocation(voxelShader.ID, "camera_pos"), cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform3f(glGetUniformLocation(voxelShader.ID, "camera_dir"), cameraTarget.x, cameraTarget.y, cameraTarget.z);
    glUniform3f(glGetUniformLocation(voxelShader.ID, "camera_up"), upVector.x, upVector.y, upVector.z);
    glUniform2f(glGetUniformLocation(voxelShader.ID, "resolution"), 1920, 1080);

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