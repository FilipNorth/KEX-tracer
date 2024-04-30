#version 460 core

#define INV_STEP_LENGTH (1.0f/STEP_LENGTH)
#define STEP_LENGTH 0.005f

layout(binding = 0, rgba8) uniform image3D voxelTexture;
uniform vec3 camPos;
uniform int state = 0;
uniform mat4 invViewProj;

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normalMap0;
uniform sampler2D metallicRoughnessMap0;

in vec2 textureCoordinateFrag;
in vec3 PositionFrag;
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
    float stepSize = 0.5 * (64.0 / float(textureSize));  // Adjust step size based on texture size
    float maxDistance = float(textureSize);  // Assume maxDistance correlates to texture size

    vec4 accumulatedColor = vec4(0.0);
    float accumulatedAlpha = 0.0;

    for (float dist = 0.0; dist < maxDistance; dist += stepSize) {
        vec3 currentPos = rayOrigin + dist * rayDir;
        ivec3 voxelCoord = ivec3(currentPos * (float(textureSize) / maxDistance));  // Scale position to voxel grid

        if (any(lessThan(voxelCoord, ivec3(0))) || any(greaterThanEqual(voxelCoord, ivec3(textureSize)))) {
            continue;
        }

        vec4 voxelData = imageLoad(voxelTexture, voxelCoord);
        voxelData.rgb *= voxelData.a;

        accumulatedColor += (1.0 - accumulatedAlpha) * voxelData;
        accumulatedAlpha += (1.0 - accumulatedAlpha) * voxelData.a;

        if (accumulatedAlpha >= 0.95) {
            break;
        }
    }

    FragColor = accumulatedColor;
}