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
uniform sampler2D MaskTexture;
uniform sampler2D HeightTexture;
uniform vec2 HeightTextureSize;


// Material properties
uniform float Shininess;
uniform float Opacity;

// Shadow map
uniform sampler2DShadow ShadowMap;

// Voxel stuff
uniform sampler3D VoxelTexture;
uniform float VoxelGridWorldSize;
uniform int VoxelDimensions;

// Bounce stuff
uniform layout(RGBA8) image3D VoxelTextureBounce;

// Toggle "booleans"
uniform float ShowDiffuse;
uniform float ShowIndirectDiffuse;
uniform float ShowIndirectSpecular;
uniform float ShowAmbientOcculision;

uniform vec3 LightDirection;

const float MAX_DIST = 100.0;
const float ALPHA_THRESH = 0.95;

const int DIFFUSE_CONE_COUNT = 16;
const float DIFFUSE_CONE_APERTURE = 0.872665;


// const int NUM_CONES = 16;
// const vec3 coneDirections[16] = {
//     vec3(0.57735, 0.57735, 0.57735),
//     vec3(0.57735, -0.57735, -0.57735),
//     vec3(-0.57735, 0.57735, -0.57735),
//     vec3(-0.57735, -0.57735, 0.57735),
//     vec3(-0.903007, -0.182696, -0.388844),
//     vec3(-0.903007, 0.182696, 0.388844),
//     vec3(0.903007, -0.182696, 0.388844),
//     vec3(0.903007, 0.182696, -0.388844),
//     vec3(-0.388844, -0.903007, -0.182696),
//     vec3(0.388844, -0.903007, 0.182696),
//     vec3(0.388844, 0.903007, -0.182696),
//     vec3(-0.388844, 0.903007, 0.182696),
//     vec3(-0.182696, -0.388844, -0.903007),
//     vec3(0.182696, 0.388844, -0.903007),
//     vec3(-0.182696, 0.388844, 0.903007),
//     vec3(0.182696, -0.388844, 0.903007)
// };
// float coneWeights[16] = float[](0.0625, 0.0625, 0.0625, 0.0625,  0.0625, 0.0625, 0.0625, 0.0625, 0.0625, 0.0625, 0.0625, 0.0625, 0.0625, 0.0625, 0.0625, 0.0625);

// 6 60 degree cone
const int NUM_CONES = 6;
vec3 coneDirections[6] = vec3[]
(                            vec3(0, 1, 0),
                            vec3(0, 0.5, 0.866025),
                            vec3(0.823639, 0.5, 0.267617),
                            vec3(0.509037, 0.5, -0.700629),
                            vec3(-0.509037, 0.5, -0.700629),
                            vec3(-0.823639, 0.5, 0.267617)
                            );
float coneWeights[6] = float[](0.25, 0.15, 0.15, 0.15, 0.15, 0.15);

// // 5 90 degree cones
// const int NUM_CONES = 5;
// vec3 coneDirections[5] = vec3[]
// (                            vec3(0, 1, 0),
//                             vec3(0, 0.707, 0.707),
//                             vec3(0, 0.707, -0.707),
//                             vec3(0.707, 0.707, 0),
//                             vec3(-0.707, 0.707, 0)
//                             );
// float coneWeights[5] = float[](0.28, 0.18, 0.18, 0.18, 0.18);

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

mat3 tangentToWorld;

vec4 sampleVoxels(vec3 worldPosition, float lod) {
    vec3 offset = vec3(1.0 / VoxelDimensions, 1.0 / VoxelDimensions, 0); // Why??
    vec3 voxelTextureUV = worldPosition / (VoxelGridWorldSize * 0.5);
    voxelTextureUV = voxelTextureUV * 0.5 + 0.5 + offset;
    return textureLod(VoxelTexture, voxelTextureUV, lod);
}

// Third argument to say how long between steps?
vec4 coneTrace(vec3 direction, float tanHalfAngle, out float occlusion) {
    
    // lod level 0 mipmap is full size, level 1 is half that size and so on
    float lod = 0.0;
    vec3 color = vec3(0);
    float alpha = 0.0;
    occlusion = 0.0;

    float voxelWorldSize = VoxelGridWorldSize / VoxelDimensions;
    float dist = voxelWorldSize; // Start one voxel away to avoid self occlusion
    vec3 startPos = Position_world + Normal_world * voxelWorldSize; // Plus move away slightly in the normal direction to avoid
                                                                    // self occlusion in flat surfaces

    while(dist < MAX_DIST && alpha < ALPHA_THRESH) {
        // smallest sample diameter possible is the voxel size
        float diameter = max(voxelWorldSize, 2.0 * tanHalfAngle * dist);
        float lodLevel = log2(diameter / voxelWorldSize);
        vec4 voxelColor = sampleVoxels(startPos + dist * direction, lodLevel);

        // front-to-back compositing
        float a = (1.0 - alpha);
        color += a * voxelColor.rgb;
        alpha += a * voxelColor.a;
        //occlusion += a * voxelColor.a;
        occlusion += (a * voxelColor.a) / (1.0 + 0.03 * diameter);
        dist += diameter * 1.0; // smoother
        //dist += diameter; // faster but misses more voxels
    }

    return vec4(color, alpha);
}

vec4 indirectLight(out float occlusion_out) {
    vec4 color = vec4(0);
    occlusion_out = 0.0;

    for(int i = 0; i < NUM_CONES; i++) {
        float occlusion = 0.0;
        // 22.5 degree cones -> tan(22.5/2) = 0.198
        // 60 degree cones -> tan(30) = 0.577
        // 90 degree cones -> tan(45) = 1.0
        color += coneWeights[i] * coneTrace(tangentToWorld * coneDirections[i], 0.577, occlusion);
        occlusion_out += coneWeights[i] * occlusion;
    }

    occlusion_out = 1.0 - occlusion_out;

    return color;
}

vec3 calcBumpNormal() {
    // Fetch the encoded normal from the normal map texture
    vec3 encodedNormal = texture(normalMap, UV).rgb;

    // Convert from [0, 1] range to [-1, 1] range
    vec3 tangentNormal = encodedNormal * 2.0 - 1.0;

    // Assuming your tangentNormal is in tangent space, you need to transform it to world space
    // If you have a TBN matrix available (from tangent, bitangent, and normal), you can use it:
    return normalize(tangentToWorld * tangentNormal);

    // If tangentToWorld is the transformation matrix from tangent space to world space.
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec4 materialColor = texture(baseColorTexture, UV);
    vec4 metallicRoughnessColor = texture(metallicRoughnessTexture, UV);
    float roughness = metallicRoughnessColor.g;
    float metallic = metallicRoughnessColor.b;
    vec4 diffuseColor = materialColor;
    float alpha = materialColor.a;

    if(alpha < 0.5) {
        discard;
    }
    
    //tangentToWorld = inverse(transpose(mat3(Tangent_world, Normal_world, Bitangent_world)));
    tangentToWorld = mat3(Tangent_world, Bitangent_world, Normal_world); // Ensure proper orientation and handedness


    // Normal, light direction and eye direction in world coordinates
    vec3 N = calcBumpNormal();
    vec3 L = LightDirection;
    vec3 E = normalize(EyeDirection_world);
    
    // Calculate diffuse light
    vec3 diffuseReflection;
    {
        // Shadow map
        float visibility = texture(ShadowMap, vec3(Position_depth.x, Position_depth.y, (Position_depth.z - 0.0005)/Position_depth.w));
        // Direct diffuse light
        float cosTheta = max(0, dot(N, L));
        vec3 directDiffuseLight = ShowDiffuse > 0.5 ? vec3(visibility * cosTheta) : vec3(0.0);

        // Indirect diffuse light
		float occlusion = 0.0;
        vec3 indirectDiffuseLight = indirectLight(occlusion).rgb;
        indirectDiffuseLight = ShowIndirectDiffuse > 0.5 ? 4.0 * indirectDiffuseLight * roughness : vec3(0.0);

        // Sum direct and indirect diffuse light and tweak a little bit
        occlusion = min(1.0,  0.5 * occlusion); // Make occlusion brighter
        //diffuseReflection = 2.0 * occlusion * (directDiffuseLight + indirectDiffuseLight * 0.7) * materialColor.rgb;
        diffuseReflection = ( (2.0 * occlusion * indirectDiffuseLight)  +  (0.3*directDiffuseLight) ) * diffuseColor.rgb + 0.05 * diffuseColor.rgb ;
    }
    
    // Calculate specular light
    vec3 specularReflection;
    {
        ///vec4 specularColor = texture(SpecularTexture, UV);
        // Some specular textures are grayscale:
        //specularColor = length(specularColor.gb) > 0.0 ? specularColor : specularColor.rrra;
        vec3 reflectDir = normalize(reflect(-E, N));

        // Maybe fix so that the cone doesnt trace below the plane defined by the surface normal.
        // For example so that the floor doesnt reflect itself when looking at it with a small angle
        float specularOcclusion;
        float angle = roughness; // Look into what constants to use. Roughness gives angle of specular cone
        vec4 tracedSpecular = coneTrace(reflectDir, angle, specularOcclusion); // 0.2 = 22.6 degrees, 0.1 = 11.4 degrees, 0.07 = 8 degrees angle
        specularReflection = ShowIndirectSpecular > 0.5 ? 0.2 *  tracedSpecular.rgb : vec3(0.0);
    }

    color = vec4(diffuseReflection + specularReflection, alpha);
	//color = vec4(N, 1) * vec4(0.29, 0.99, 0.39, 0.0);

    //imageStore(VoxelTextureBounce, ivec3(gl_FragCoord.x, gl_FragCoord.y, VoxelDimensions * gl_FragCoord.z), vec4(color.rgb, 1.0));
}