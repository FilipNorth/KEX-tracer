#version 460 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_world;
in vec3 Normal_world;
in vec3 Tangent_world;
in vec3 Bitangent_world;
in vec3 EyeDirection_world;
in vec4 Position_depth; // Position from the shadow map point of view

out vec4 color;

// Textures
uniform sampler2D baseColorTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D normalMap;
uniform sampler2DShadow ShadowMap;

// Voxel and lighting
uniform sampler3D VoxelTexture;
uniform float VoxelGridWorldSize;
uniform int VoxelDimensions;

// Toggle "booleans"
uniform float ShowDiffuse;
uniform float ShowIndirectDiffuse;
uniform float ShowIndirectSpecular;

uniform vec3 LightDirection;

const float PI = 3.14159265359;
const float MAX_DIST = 100.0;
const float ALPHA_THRESH = 0.95;

// Directional and occlusion data for cones
const int NUM_CONES = 6;
vec3 coneDirections[6] = vec3[]
(   vec3(0, 1, 0),
    vec3(0, 0.5, 0.866025),
    vec3(0.823639, 0.5, 0.267617),
    vec3(0.509037, 0.5, -0.700629),
    vec3(-0.509037, 0.5, -0.700629),
    vec3(-0.823639, 0.5, 0.267617)
);
float coneWeights[6] = float[](0.25, 0.15, 0.15, 0.15, 0.15, 0.15);

mat3 tangentToWorld;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calcBumpNormal() {
    vec3 encodedNormal = texture(normalMap, UV).rgb;
    vec3 tangentNormal = encodedNormal * 2.0 - 1.0;
    return normalize(tangentToWorld * tangentNormal);
}

vec4 sampleVoxels(vec3 worldPosition, float lod) {
    vec3 voxelTextureUV = worldPosition / VoxelGridWorldSize + 0.5;
    return textureLod(VoxelTexture, voxelTextureUV, lod);
}

vec4 coneTrace(vec3 direction, float tanHalfAngle, out float occlusion) {
    float lod = 0.0;
    vec3 color = vec3(0);
    float alpha = 0.0;
    occlusion = 0.0;

    float voxelWorldSize = VoxelGridWorldSize / VoxelDimensions;
    float dist = voxelWorldSize;
    vec3 startPos = Position_world + Normal_world * voxelWorldSize;

    while(dist < MAX_DIST && alpha < ALPHA_THRESH) {
        float diameter = max(voxelWorldSize, 2.0 * tanHalfAngle * dist);
        float lodLevel = log2(diameter / voxelWorldSize);
        vec4 voxelColor = sampleVoxels(startPos + dist * direction, lodLevel);

        float a = (1.0 - alpha);
        color += a * voxelColor.rgb;
        alpha += a * voxelColor.a;
        occlusion += a * voxelColor.a / (1.0 + 0.03 * diameter);
        dist += diameter;
    }
    return vec4(color, alpha);
}

vec4 indirectLight(out float occlusion_out) {
    vec4 color = vec4(0);
    occlusion_out = 0.0;
    for(int i = 0; i < NUM_CONES; i++) {
        float occlusion = 0.0;
        color += coneWeights[i] * coneTrace(tangentToWorld * coneDirections[i], 0.577, occlusion);
        occlusion_out += coneWeights[i] * occlusion;
    }
    occlusion_out = 1.0 - occlusion_out;
    return color;
}

void main() {
    vec4 baseColor = texture(baseColorTexture, UV);
    vec4 mr = texture(metallicRoughnessTexture, UV);
    float roughness = mr.g;
    float metallic = mr.b;
    float alpha = baseColor.a;

    if(alpha < 0.5) {
        discard;
    }

    tangentToWorld = mat3(Tangent_world, Bitangent_world, Normal_world);
    vec3 N = calcBumpNormal();
    vec3 V = normalize(EyeDirection_world);
    vec3 L = normalize(LightDirection);
    vec3 H = normalize(V + L);

    vec3 F0 = mix(vec3(0.04), baseColor.rgb, metallic);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Indirect lighting via cone tracing
    float occlusion;
    vec3 indirectDiffuse = ShowIndirectDiffuse > 0.5 ? indirectLight(occlusion).rgb : vec3(0.0);
    vec3 indirectSpecular = ShowIndirectSpecular > 0.5 ? coneTrace(N, 0.1, occlusion).rgb : vec3(0.0); // Simplified specular cone trace

    vec3 colorResult = indirectDiffuse + indirectSpecular;
    color = vec4(colorResult, alpha);
}
