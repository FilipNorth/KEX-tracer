#version 460 core
layout(binding = 0, rgba8) uniform image3D voxelTexture;
uniform ivec3 gridSize;
out vec4 FragColor;
in vec4 debugColor;
in vec3 worldPositionFrag;
in vec3 normalFrag;

void main() {
	vec3 voxelCoord = normalFrag * 0.5 + vec3(0.5);
    ivec3 voxelPos = ivec3(voxelCoord * gridSize);
    imageStore(voxelTexture, voxelPos, vec4(0.0, 0.0, 1.0, 1.0));
	ivec3 sz = imageSize(voxelTexture); // This ends up being ivec3(0, 0, 0)

	if(sz.x == 0 && sz.y == 0 && sz.z == 0){
		FragColor = vec4(0.04, 0.83, 0.27, 1.0);
	}
	else{
    	FragColor = vec4(vec3(sz.rgb), 1.0);
	}
}
