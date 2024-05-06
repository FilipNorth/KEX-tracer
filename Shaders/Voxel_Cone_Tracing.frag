#version 460 core

#define INV_STEP_LENGTH (1.0f/STEP_LENGTH)
#define STEP_LENGTH 0.1f

layout(binding = 0, rgba8) uniform image3D voxelTexture;
uniform vec3 camPos;
uniform int state = 0;
uniform mat4 invViewProj;
uniform float worldSize = 20.0;  // Define the size of the world volume covered by the voxel grid
uniform vec3 viewDirection;


in vec2 textureCoordinateFrag;
out vec4 FragColor;

vec3 getRayDirection(vec2 screenCoords, mat4 invViewProjMatrix) {
    vec4 clipCoords = vec4(screenCoords * 2.0 - 1.0, 1.0, 1.0);
    vec4 viewCoords = invViewProjMatrix * clipCoords;
    vec3 dir = viewCoords.xyz / viewCoords.w;
    return length(dir) > 0.0001 ? normalize(dir) : vec3(0.0, 0.0, 0.0);
}

float calculateAttenuation(vec3 lightPos, vec3 pointPos) {
    float distance = length(lightPos - pointPos);
    return 1.0 / (1.0 + 0.1 * distance * distance);  // Quadratic falloff; adjust constant as needed
}


vec4 calculateLighting(vec3 position, vec3 normal, vec3 viewDir, vec3 lightPos, vec4 lightCol, float lightIntensity) {
    // Normalize the light direction vector and check for zero-length to avoid undefined behavior
    vec3 lightVector = normalize(lightPos - position);
    vec3 lightDir = length(lightVector) > 0.0001 ? normalize(lightVector) : vec3(0.0, 0.0, 0.0);

    // Calculate attenuation
    float attenuation = calculateAttenuation(lightPos, position);

    // Calculate diffuse component
    float diff = max(dot(normal, lightDir), 0.0);

    // Calculate the reflection direction for specular lighting
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Ensure viewDir is normalized before using it

    // Components of lighting
    vec4 ambient = 0.1 * lightCol; // Ambient light component
    vec4 diffuse = diff * lightCol * attenuation; // Diffuse light component
    vec4 specular = spec * vec4(1.0) * attenuation; // Specular component, assuming white specular color

    return lightIntensity * (diffuse);
}


vec3 calculateNormal(vec3 currentPos, vec3 previousPos, int textureSize, float worldSize) {
    vec3 step = (currentPos - previousPos); // Difference between the current and previous positions

    // Normalizing the position to voxel coordinates
    vec3 currentNormalizedPosition = (currentPos + vec3(worldSize / 2.0)) / worldSize;
    vec3 previousNormalizedPosition = (previousPos + vec3(worldSize / 2.0)) / worldSize;

    ivec3 currentVoxelCoord = ivec3(currentNormalizedPosition * float(textureSize));
    ivec3 previousVoxelCoord = ivec3(previousNormalizedPosition * float(textureSize));

    // Calculate gradients using central differences
    vec3 normal = vec3(0.0);
    if (any(notEqual(currentVoxelCoord, previousVoxelCoord))) { // Ensure we are not comparing the same voxel
        vec3 dir = normalize(step);
        vec3 absDir = abs(dir);

        // Determine the dominant direction of travel between steps
        if (absDir.x > absDir.y && absDir.x > absDir.z) {
            normal = vec3(sign(dir.x), 0.0, 0.0);
        } else if (absDir.y > absDir.z) {
            normal = vec3(0.0, sign(dir.y), 0.0);
        } else {
            normal = vec3(0.0, 0.0, sign(dir.z));
        }
    } else {
        // Fallback to a simple approximation if no movement between voxel coordinates
        normal = normalize(cross(step, vec3(0.0, 0.0, 1.0))); // Use arbitrary vector for cross product
    }

    return normal;
}


vec3 getNormalFromHitPoint(vec3 hitPoint, vec3 voxelCenter, vec3 previousPoint) {
    //vec3 dir = (hitPoint + previousPoint) / 2 - voxelCenter; 
    vec3 dir = hitPoint - voxelCenter;
    vec3 absDir = abs(dir);
    vec3 normal = vec3(0.0);

    if (absDir.x > absDir.y && absDir.x > absDir.z) {
        normal = vec3(sign(dir.x), 0.0, 0.0);
    } else if (absDir.y > absDir.z) {
        normal = vec3(0.0, sign(dir.y), 0.0);
    } else {
        normal = vec3(0.0, 0.0, sign(dir.z));
    }

    return normal;
}

vec4 rayMarching(vec3 rayOrigin, vec3 rayDir, float maxDistance, float normalizedStepSize, int textureSize, float worldSize) {
    vec4 accumulatedColor = vec4(0.0);
    float accumulatedAlpha = 0.0;
    vec3 lightPos = vec3(0.0, 5.0, 1.0);
    float lightRadius = 0.1;
    vec3 normalizedLightPos = normalize(lightPos);
    vec3 voxelizedLightPos = vec3(normalizedLightPos * textureSize);

    vec4 lightColor = vec4(1);
    float lightIntensity = 1;

    for (float dist = 0.0; dist < maxDistance; dist += normalizedStepSize) {
        vec3 currentPos = rayOrigin + dist * rayDir;
        vec3 normalizedPosition = (currentPos + (worldSize / 2.0)) / worldSize;
        ivec3 voxelCoord = ivec3(normalizedPosition * textureSize);

        if (any(lessThan(voxelCoord, ivec3(0))) || any(greaterThanEqual(voxelCoord, ivec3(textureSize)))) {
            continue;
        }

        // Calculate distance from current voxel to the light position
        vec3 worldPos = (vec3(voxelCoord) / textureSize) * worldSize - vec3(worldSize / 2.0);
        if (length(worldPos - lightPos) < lightRadius) {
            return vec4(1.0, 1.0, 1.0, 1.0);  // Visualize light sphere
        }

        vec4 voxelData = imageLoad(voxelTexture, voxelCoord);
        if (voxelData.a > 0.1) { // Consider as surface if alpha is significant
            vec3 previousPoint = currentPos - normalizedStepSize * rayDir;
            vec3 normalizedPreviousPoint = (previousPoint + (worldSize / 2.0)) / worldSize;
            vec3 normal = getNormalFromHitPoint(normalizedPosition * textureSize, voxelCoord, normalizedPreviousPoint * textureSize);
            //return vec4(normal, 1);
            //vec3 normal = CalcNormal(currentPos - normalizedStepSize * rayDir, textureSize, worldSize, normalizedStepSize, rayDir);
            //return FragColor = vec4((normal * 0.5) + 0.5, 1.0); // Convert from [-1, 1] to [0, 1]
            vec3 viewDir = normalize(camPos - currentPos); // Vector from point to camera
            viewDir = viewDirection;
            //if(viewDir.x < 0.01 && viewDir.y < 0.01 && viewDir.z < 0.01){
            //    return vec4(normalize(camPos), 1.0);
            //}
            //return vec4(viewDir, 1);
            //return vec4(normalize(lightPos - currentPos), 1);

            vec4 lighting = calculateLighting(currentPos, normal, viewDir, lightPos, lightColor, lightIntensity);
            accumulatedColor += (1.0 - accumulatedAlpha) * voxelData * lighting;
            accumulatedAlpha += (1.0 - accumulatedAlpha) * voxelData.a;
        }

        if (accumulatedAlpha >= 0.95) {
            break;
        }
    }

    return accumulatedColor;
}


void main() {
    vec3 rayDir = getRayDirection(textureCoordinateFrag, invViewProj);
    vec3 rayOrigin = camPos;

    int textureSize = imageSize(voxelTexture).x;  // Assuming the texture is cubic
    float normalizedStepSize = (worldSize / float(textureSize)) * 0.1;  // Normalize the step size
    float maxDistance = worldSize;  // Max distance in normalized world coordinates

    vec4 accumulatedColor = vec4(0.0);
    float accumulatedAlpha = 0.0;

    accumulatedColor = rayMarching(rayOrigin, rayDir, maxDistance, normalizedStepSize, textureSize, worldSize);
    

    FragColor = vec4(accumulatedColor.rgb, accumulatedColor.a);
    // Outputting depth component as color for debugging
    //FragColor = vec4(rayDir, 1);

}
