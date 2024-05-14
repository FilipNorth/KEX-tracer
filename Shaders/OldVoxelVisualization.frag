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

#define NUM_DIFFUSE_CONES 5
const vec3 ConeVectors[5] = vec3[5](
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.707106781, 0.707106781),
    vec3(0.0, -0.707106781, 0.707106781),
    vec3(0.707106781, 0.0, 0.707106781),
    vec3(-0.707106781, 0.0, 0.707106781)
);

const float Weights[5] = float[5](0.28, 0.18, 0.18, 0.18, 0.18);
const float Apertures[5] = float[5]( /* tan(45) */ 1.0, 1.0, 1.0, 1.0, 1.0 );
const float shadowAngle = radians(30.0);

vec3 getRayDirection(vec2 screenCoords, mat4 invViewProjMatrix) {
    vec4 clipCoords = vec4(screenCoords * 2.0 - 1.0, 1.0, 1.0);
    vec4 viewCoords = invViewProjMatrix * clipCoords;
    //return viewCoords.xyz;
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
    if(lightVector == vec3(0.0, 0.0, 0.0)){
        return vec4(1.0, 0.0, 0.0, 1.0);
    }
    vec3 lightDir = length(lightVector) > 0.0001 ? normalize(lightVector) : vec3(0.0, 0.0, 0.0);

    // Calculate attenuation
    float attenuation = calculateAttenuation(lightPos, position);

    vec3 testNormal = vec3(-1.0, 0.0, 0.0);
    // Calculate diffuse component
    float diff = max(dot(normal, lightVector), 0.0); // Ensure normal and lightDir are normalized before using them
    // Calculate the reflection direction for specular lighting
    vec3 reflectDir = reflect(-lightVector, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Ensure viewDir is normalized before using it

    // Components of lighting
    vec4 ambient = 0.1 * lightCol; // Ambient light component
    vec4 diffuse = diff * lightCol * attenuation; // Diffuse light component
    vec4 specular = spec * vec4(1.0) * attenuation; // Specular component, assuming white specular color

    return lightIntensity * (diffuse  + ambient);  // Return the sum of all components
}

vec3 getNormalFromHitPoint(vec3 hitPoint, vec3 previousHitPoint, ivec3 voxelCenter) {
    //vec3 dir = (hitPoint + previousPoint) / 2 - voxelCenter; 
    ivec3 voxelCoord = ivec3(hitPoint);
    ivec3 previousVoxelCoord = ivec3(previousHitPoint);
    vec3 normal = vec3(0.0);
    //return previousVoxelCoord - voxelCoord;
    vec3 nonNormalDiff = vec3(previousVoxelCoord - voxelCoord);
    vec3 coordDiff = abs(vec3(previousVoxelCoord - voxelCoord));
    vec3 absCoordDiff = abs(coordDiff);
    if (coordDiff.x > coordDiff.y && coordDiff.x > coordDiff.z) {
        normal = vec3(sign(nonNormalDiff.x), 0.0, 0.0);
    } else if (coordDiff.y > coordDiff.z) {
        normal = vec3(0.0, sign(nonNormalDiff.y), 0.0);
    } else {
        normal = vec3(0.0, 0.0, sign(nonNormalDiff.z));
    }


    if(coordDiff.x > 0.1|| coordDiff.y > 0.1 ||coordDiff.z > 0.1){
       return normal;
    }
    else{
        return vec3(1.0, 1.0, 1.0);
    }

   /// return vec3(1, 1, 1);

    vec3 dir = hitPoint - voxelCenter;
    vec3 absDir = abs(dir);

    if (absDir.x > absDir.y && absDir.x > absDir.z) {
        normal = vec3(sign(dir.x), 0.0, 0.0);
    } else if (absDir.y > absDir.z) {
        normal = vec3(0.0, sign(dir.y), 0.0);
    } else {
        normal = vec3(0.0, 0.0, sign(dir.z));
    }

    return normal;
}


bool raymarchShadows(vec3 rayOrigin, vec3 rayDir, float maxDistance, int textureSize) {
    float step = 0.1;  // Smaller step for higher precision in shadow calculation
    for (float d = 0.0; d <= maxDistance; d += step) {
        vec3 pos = rayOrigin + d * rayDir;
        vec3 normalizedPosition = (pos + (worldSize / 2.0)) / worldSize;
        ivec3 voxelCoord = ivec3(normalizedPosition * float(textureSize));
        if (any(lessThan(voxelCoord, ivec3(0))) || any(greaterThanEqual(voxelCoord, ivec3(textureSize))))
            continue;
        vec4 voxel = imageLoad(voxelTexture, voxelCoord);
        if (voxel.a > 0.1) return true; // Obstacle found, in shadow
    }
    return false; // No obstacle, not in shadow
}

vec4 coneTrace(vec3 origin, vec3 direction, float angle, float maxDistance, int steps, int textureSize) {
    vec3 stepDirection = normalize(direction) * (maxDistance / float(steps)) * 0.5;
    float coneRadiusStep = tan(angle) * length(stepDirection);
    float accumulatedOcclusion = 0.0;

    for (int i = 0; i < steps; ++i) {
        vec3 samplePoint = origin + stepDirection * float(i);
        vec3 normalizedPosition = (samplePoint + (worldSize / 2.0)) / worldSize;
        ivec3 voxelCoord = ivec3(normalizedPosition * float(textureSize));

        if (any(lessThan(voxelCoord, ivec3(0))) || any(greaterThanEqual(voxelCoord, ivec3(textureSize))))
            continue;

        vec4 voxelData = imageLoad(voxelTexture, voxelCoord);
        if (voxelData.a > 0.0) {
            float occlusionIncrement = voxelData.a * (1.0 - accumulatedOcclusion);
            accumulatedOcclusion += occlusionIncrement;
            if (accumulatedOcclusion >= 0.95) break;  // Exit early if occlusion is high enough
        }
    }

    return vec4(1.0 - accumulatedOcclusion);  // Returns the light intensity not occluded
}


vec3 diffuse_cones(vec3 pos, vec3 normal, vec3 tangent, vec3 bitangent, int textureSize, float voxel_size) {
    mat3 space = mat3(tangent, bitangent, normal);
    vec3 accumulatedLight = vec3(0);

    for (int i = 0; i < NUM_DIFFUSE_CONES; i++) {
        vec3 dir = space * normalize(ConeVectors[i]);
        vec4 coneResult = coneTrace(pos + normal * 1.75 * voxel_size, dir, Apertures[i], 10, 100, textureSize);
        accumulatedLight += Weights[i] * coneResult.rgb * vec3(1.0);  // Using x component which is light intensity
    }

    return accumulatedLight;
}

vec3 createOrthogonalVectors(vec3 n) {
    vec3 tangent;
    if (abs(n.x) > abs(n.z)) {
        tangent = vec3(-n.y, n.x, 0.0);
    } else {
        tangent = vec3(0.0, -n.z, n.y);
    }
    return tangent = normalize(tangent);
}


vec4 rayMarching(vec3 rayOrigin, vec3 rayDir, float maxDistance, float normalizedStepSize, int textureSize, float worldSize, vec3 lightPos, vec4 lightColor, float lightRadius, int bounce) {
    vec4 accumulatedColor = vec4(0.0);
    float accumulatedAlpha = 0.0;

    
    float lightIntensity = 1.0;

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
            vec3 previousPoint = currentPos - normalizedStepSize * 2 * rayDir;
            vec3 normalizedPreviousPoint = (previousPoint + (worldSize / 2.0)) / worldSize;
            vec3 normal = getNormalFromHitPoint(normalizedPosition * textureSize, normalizedPreviousPoint * textureSize, voxelCoord);
            
            vec3 viewDir = normalize(camPos - currentPos); // Vector from point to camera
            // Cone tracing for soft shadows
                // Wider cone for softer shadows
            vec4 shadowResult = coneTrace(currentPos, normalize(lightPos - currentPos), shadowAngle, length(lightPos - currentPos), 10, textureSize);
            float shadowFactor = 1 - shadowResult.a;  // Determine shadow strength based
            vec3 tangent = createOrthogonalVectors(normal); // Declare tangent (and bitangent
            vec3 bitangent = cross(normal, tangent);
            vec3 ao_and_soft_shadows = vec3(diffuse_cones(currentPos, normal, tangent, bitangent, textureSize,  worldSize / textureSize)); // Convert result to vector
            //vec4 lighting = calculateLighting(currentPos, normal, viewDir, lightPos, lightColor, lightIntensity);
            
            //return vec4(normalize(tangent), 1.0);
            //return vec4(ao_and_soft_shadows, 1.0);
            vec4 lighting = calculateLighting(currentPos, normal, viewDir, lightPos, lightColor, lightIntensity) * shadowFactor;
        
            
           
            lighting.rgb *= ao_and_soft_shadows;  // Modulate the lighting by the AO and soft shadows

            
            viewDir = viewDirection;

            if(bounce == 0){
                vec3 lightVector = normalize(lightPos - currentPos);
                vec3 lightDir = length(lightVector) > 0.0001 ? normalize(lightVector) : vec3(0.0, 0.0, 0.0);
                vec3 reflectDir = reflect(-lightDir, normal);
            
                
            }
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
    float normalizedStepSize = (worldSize / float(textureSize)) * 0.1 ;  // Normalize the step size
    float maxDistance = worldSize;  // Max distance in normalized world coordinates

    vec4 accumulatedColor = vec4(0.0);
    float accumulatedAlpha = 0.0;

    vec3 lightPos = vec3(0.0, 1.0, 1.0);
    float lightRadius = 0.1;
    vec3 normalizedLightPos = normalize(lightPos);
    vec4 lightColor = vec4(1.0);

    accumulatedColor = rayMarching(rayOrigin, rayDir, maxDistance, normalizedStepSize, textureSize, worldSize, lightPos, lightColor, lightRadius, 1);


    FragColor = vec4(accumulatedColor.rgb, accumulatedColor.a);


}
