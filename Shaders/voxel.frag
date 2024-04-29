#version 460 core

#define INV_STEP_LENGTH (1.0f/STEP_LENGTH)
#define STEP_LENGTH 0.005f

layout(binding = 0, rgba8) uniform image3D voxelTexture;
uniform vec3 camPos; // World camera position.
uniform int state = 0; // Decides mipmap sample level.
uniform mat4 invViewProj; // Inverse of the view projection matrix.

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normalMap0;
uniform sampler2D metallicRoughnessMap0; 

in vec2 textureCoordinateFrag; 
out vec4 FragColor;

vec3 getRayDirection(vec2 screenCoords, mat4 invViewProjMatrix) {
    vec4 clipCoords = vec4(screenCoords * 2.0 - 1.0, 1.0, 1.0);  // Convert to clip space
    vec4 viewCoords = invViewProjMatrix * clipCoords;  // Convert to view space
    return normalize(viewCoords.xyz / viewCoords.w);  // Normalize the direction
}

void main() {
    vec3 rayDir = getRayDirection(textureCoordinateFrag, invViewProj);
    vec3 rayOrigin = camPos;  // Use the camera position as the ray origin

    float stepSize = 0.5;  // Smaller step size for better quality
    float maxDistance = 64.0;  // Maximum distance to ray march
    vec4 accumulatedColor = vec4(0.0);
    float accumulatedAlpha = 0.0;

    for (float dist = 0.0; dist < maxDistance; dist += stepSize) {
        vec3 currentPos = rayOrigin + dist * rayDir;  // Current position along the ray
        ivec3 voxelCoord = ivec3(currentPos);  // Convert to voxel coordinates
        
        // Check if the coordinate is within the bounds of the voxel texture
        if (any(lessThan(voxelCoord, ivec3(0))) || any(greaterThanEqual(voxelCoord, imageSize(voxelTexture)))) {
            continue;  // Skip samples outside the texture bounds
        }

        vec4 voxelData = imageLoad(voxelTexture, voxelCoord);  // Load voxel data
        voxelData.rgb *= voxelData.a;  // Pre-multiply alpha for correct compositing

        // Front-to-back compositing
        accumulatedColor += (1.0 - accumulatedAlpha) * voxelData;
        accumulatedAlpha += (1.0 - accumulatedAlpha) * voxelData.a;

        if (accumulatedAlpha >= 0.95) {  // Early termination for performance
            break;
        }
    }

    FragColor = accumulatedColor;
}