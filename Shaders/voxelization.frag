#version 460 core
#extension GL_ARB_shader_image_load_store : enable
#extension GL_ARB_shader_storage_buffer_object : enable

// Data from geometry shader
in fData {
    vec2 UV;
    flat int axis;
    vec4 position_depth; // Position from the shadow map point of view
	vec3 normal;
} frag;

layout(std430, binding = 4) buffer PageFlags {
    uint flags[];
};

uniform layout(binding = 0, RGBA8) image3D VoxelTexture;
//uniform layout(binding = 2, RGBA16F) image3D VoxelNormalTexture;
uniform sampler2D diffuse;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D normalMap;
uniform sampler2DShadow ShadowMap;
uniform int VoxelDimensions;

void main() {

    vec4 materialColor = texture(diffuse, frag.UV);

    // Do shadow map lookup here
    // TODO: Splat photons onto the voxels at a later stage using a separate shader
    float visibility = texture(ShadowMap, vec3(frag.position_depth.xy, (frag.position_depth.z - 0.001)/frag.position_depth.w));

	ivec3 camPos = ivec3(gl_FragCoord.x, gl_FragCoord.y, VoxelDimensions * gl_FragCoord.z);
	ivec3 texPos;
	if(frag.axis == 1) {
	    texPos.x = VoxelDimensions - camPos.z;
		texPos.z = camPos.x;
		texPos.y = camPos.y;
	}
	else if(frag.axis == 2) {
	    texPos.z = camPos.y;
		texPos.y = VoxelDimensions - camPos.z;
		texPos.x = camPos.x;
	} else {
	    texPos = camPos;
	}

	// Flip it!
	texPos.z = VoxelDimensions - texPos.z - 1;
	// Overwrite currently stored value.
	// TODO: Atomic operations to get an averaged value, described in OpenGL insights about voxelization
	// Required to avoid flickering when voxelizing every frame
    
	imageStore(VoxelTexture, texPos, vec4(materialColor.rgb * visibility, 1.0));
	
	
	//imageStore(VoxelNormalTexture, texPos, vec4(frag.normal.x, frag.normal.y, frag.normal.z, roughness));
	    // Calculate page index (assuming pages are 32x32x32 voxels)

}