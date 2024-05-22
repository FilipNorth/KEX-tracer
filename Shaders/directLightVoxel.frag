#version 460 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_world;
in vec3 Normal_world;
in vec4 Position_depth; // Position from the shadow map point of view

out vec4 color;


// Textures
uniform sampler2D baseColorTexture;


// Material properties
uniform float Shininess;
uniform float Opacity;

// Shadow map
uniform sampler2DShadow ShadowMap;

// Voxel stuff
//uniform sampler3D VoxelTexture;
uniform float VoxelGridWorldSize;
uniform int VoxelDimensions;

uniform mat4 ModelMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;

// Bounce stuff
uniform layout(RGBA8) image3D VoxelTexture;

// Toggle "booleans"
uniform float ShowDiffuse;
uniform float SaveLightToVoxel;

uniform vec3 LightDirection;


void main() {
    vec4 materialColor = texture(baseColorTexture, UV);


    float alpha = materialColor.a;

    //if(alpha < 0.5) {
    //    discard;
    //}
    vec3 L = LightDirection;
    float visibility = texture(ShadowMap, vec3(Position_depth.x, Position_depth.y, (Position_depth.z - 0.0005)/Position_depth.w));

    //if(visibility < 0.01) {
   //     discard;
    //}
    // Calculate diffuse light

            // Direct diffuse light
    float cosTheta = max(0, dot(Normal_world, L));
    vec3 directDiffuseLight = vec3(visibility * cosTheta);

    vec3 startPos = Position_world;
    vec3 voxelTextureUV = startPos / (VoxelGridWorldSize * 0.5);
    voxelTextureUV = voxelTextureUV * 0.5 + 0.5;
    ivec3 StorePos = ivec3(voxelTextureUV * VoxelDimensions);

	// Overwrite currently stored value.
	// TODO: Atomic operations to get an averaged value, described in OpenGL insights about voxelization
	// Required to avoid flickering when voxelizing every frame
    
    imageStore(VoxelTexture, StorePos, vec4(directDiffuseLight * materialColor.rgb , alpha));
    
    //color = vec4(diffuseReflection + specularReflection, alpha);


	//color = vec4(N, 1) * vec4(0.29, 0.99, 0.39, 0.0);

    //imageStore(VoxelTextureBounce, ivec3(gl_FragCoord.x, gl_FragCoord.y, VoxelDimensions * gl_FragCoord.z), vec4(color.rgb, 1.0));
}