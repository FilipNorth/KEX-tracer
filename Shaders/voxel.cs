#version 460 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
};

layout (std430, binding = 0) buffer Pos {
    Vertex vertices[];
};

uniform int numVertices;
uniform vec3 gridOrigin; // Lower corner of the grid
uniform float voxelSize; // Size of each voxel
uniform int gridWidth, gridHeight, gridDepth; // Dimensions of the grid

layout (r8, binding = 1) uniform image3D voxelGrid;

void main() {
    uint idx = gl_GlobalInvocationID.x + 
               gl_GlobalInvocationID.y * gl_WorkGroupSize.x * gridWidth +
               gl_GlobalInvocationID.z * gl_WorkGroupSize.x * gridWidth * gridHeight;
    if (idx >= numVertices) return;

    vec3 pos = vertices[idx].position;
    ivec3 voxelIndex = ivec3((pos - gridOrigin) / voxelSize);

    if (voxelIndex.x >= 0 && voxelIndex.y >= 0 && voxelIndex.z >= 0 &&
        voxelIndex.x < gridWidth && voxelIndex.y < gridHeight && voxelIndex.z < gridDepth) {
        imageStore(voxelGrid, voxelIndex, vec4(1.0));
    }
}
