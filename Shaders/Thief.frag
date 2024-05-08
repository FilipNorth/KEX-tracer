#version 460 core

#define INV_STEP_LENGTH (1.0f / STEP_LENGTH)
#define STEP_LENGTH 0.1f
#define EPSILON 0.005f
#define MIPMAP_HARDCAP 5.4f
#define VOXEL_SIZE (1 / 64.0)
#define SHADOWS 1
#define DIFFUSE_INDIRECT_FACTOR 0.52f
#define SPECULAR_MODE 1
#define SPECULAR_FACTOR 4.0f
#define SPECULAR_POWER 65.0f
#define DIRECT_LIGHT_INTENSITY 0.96f
#define MAX_LIGHTS 1
#define DIST_FACTOR 1.1f
#define CONSTANT 1
#define LINEAR 0
#define QUADRATIC 1
#define GAMMA_CORRECTION 1

layout(binding = 0, rgba8) uniform image3D voxelTexture;
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normalMap0;
uniform sampler2D metallicRoughnessMap0;
uniform vec3 camPos;
uniform int state;
uniform mat4 invViewProj;
uniform float worldSize;

uniform vec3 viewDirection;

in vec3 worldPositionFrag;
in vec3 normalFrag;

out vec4 FragColor;

vec3 getRayDirection(vec2 screenCoords, mat4 invViewProjMatrix) {
    vec4 clipCoords = vec4(screenCoords * 2.0 - 1.0, 1.0, 1.0);
    vec4 viewCoords = invViewProjMatrix * clipCoords;
    return normalize(viewCoords.xyz / viewCoords.w);
}

vec3 calculateNormalFromTexture(vec2 uv) {
    return normalize(texture(normalMap0, uv).rgb * 2.0 - 1.0);
}

float attenuate(float dist) {
    dist *= DIST_FACTOR;
    return 1.0f / (CONSTANT + LINEAR * dist + QUADRATIC * dist * dist);
}

vec3 scaleAndBias(const vec3 p) {
    return 0.5f * p + vec3(0.5f);
}

vec3 traceDiffuseVoxelCone(const vec3 from, vec3 direction) {
    const float CONE_SPREAD = 0.325;
    vec4 acc = vec4(0.0f);
    float dist = 0.1953125;

    while (dist < 1.414213 && acc.a < 1) {
        vec3 c = from + dist * direction;
        c = scaleAndBias(c);
        float l = (1 + CONE_SPREAD * dist / VOXEL_SIZE);
        float level = log2(l);
        float ll = (level + 1) * (level + 1);
        vec4 voxel = imageLoad(voxelTexture, ivec3(c));
        acc += 0.075 * ll * voxel * pow(1 - voxel.a, 2);
        dist += ll * VOXEL_SIZE * 2;
    }
    return pow(acc.rgb * 2.0, vec3(1.5));
}

void main() {
    vec3 rayDir = getRayDirection(gl_FragCoord.xy, invViewProj);
    vec3 normal = calculateNormalFromTexture(gl_FragCoord.xy / vec2(textureSize(diffuse0, 0)));

    vec3 diffuseColor = texture(diffuse0, gl_FragCoord.xy / vec2(textureSize(diffuse0, 0))).rgb;
    vec3 specularColor = texture(specular0, gl_FragCoord.xy / vec2(textureSize(specular0, 0))).rgb;
    float roughness = texture(metallicRoughnessMap0, gl_FragCoord.xy / vec2(textureSize(metallicRoughnessMap0, 0))).g;
    float metalness = texture(metallicRoughnessMap0, gl_FragCoord.xy / vec2(textureSize(metallicRoughnessMap0, 0))).b;

    vec3 worldPosition = worldPositionFrag + camPos;
    vec3 indirectDiffuse = traceDiffuseVoxelCone(worldPosition, normal);

    FragColor.rgb = indirectDiffuse * diffuseColor;
    FragColor.a = 1.0;
}
