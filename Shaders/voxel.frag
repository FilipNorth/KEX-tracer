#version 460 core

#define INV_STEP_LENGTH (1.0f/STEP_LENGTH)
#define STEP_LENGTH 0.1f

layout(binding = 0, rgba8) uniform image3D voxelTexture;
uniform vec3 camPos;
uniform int state = 0;
uniform mat4 invViewProj;
uniform float worldSize = 20.0;  // Define the size of the world volume covered by the voxel grid


in vec2 textureCoordinateFrag;
out vec4 FragColor;

vec3 getRayDirection(vec2 screenCoords, mat4 invViewProjMatrix) {
    vec4 clipCoords = vec4(screenCoords * 2.0 - 1.0, 1.0, 1.0);
    vec4 viewCoords = invViewProjMatrix * clipCoords;
    return normalize(viewCoords.xyz / viewCoords.w);
}

void main() {
    vec3 rayDir = getRayDirection(textureCoordinateFrag, invViewProj);
    vec3 rayOrigin = camPos;

    int textureSize = imageSize(voxelTexture).x;  // Assuming the texture is cubic
    float normalizedStepSize = (worldSize / float(textureSize)) * 0.5;  // Normalize the step size
    float maxDistance = worldSize;  // Max distance in normalized world coordinates

    vec4 accumulatedColor = vec4(0.0);
    float accumulatedAlpha = 0.0;

    for (float dist = 0.0; dist < maxDistance; dist += normalizedStepSize) {
        vec3 currentPos = rayOrigin + dist * rayDir;
        // Normalize current position to the [0, worldSize] range
        vec3 normalizedPosition = (currentPos + (worldSize / 2.0)) / worldSize;  // Assuming the grid is centered at the origin
        ivec3 voxelCoord = ivec3(normalizedPosition * float(textureSize));  // Scale position to voxel grid size

        if (any(lessThan(voxelCoord, ivec3(0))) || any(greaterThanEqual(voxelCoord, ivec3(textureSize)))) {
            continue;  // Skip samples outside the texture bounds
        }

        vec4 voxelData = imageLoad(voxelTexture, voxelCoord);
        voxelData.rgb *= voxelData.a;  // Pre-multiply alpha for correct compositing

        accumulatedColor += (1.0 - accumulatedAlpha) * voxelData;
        accumulatedAlpha += (1.0 - accumulatedAlpha) * voxelData.a;

        if (accumulatedAlpha >= 0.95) {
            break;
        }
    }

    FragColor = accumulatedColor;
}
