#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 worldPositionGeom[];
in vec3 normalGeom[];

out vec3 worldPositionFrag;
out vec3 normalFrag;
out vec4 debugColor; // Output color for debugging

void main() {
    const vec3 p1 = worldPositionGeom[1] - worldPositionGeom[0];
    const vec3 p2 = worldPositionGeom[2] - worldPositionGeom[0];
    const vec3 p = abs(cross(p1, p2)); 

    vec3 projectedPosition;

    for(uint i = 0; i < 3; ++i) {
        worldPositionFrag = worldPositionGeom[i];
        normalFrag = normalGeom[i];

        if(p.z > p.x && p.z > p.y) {
            projectedPosition = vec3(worldPositionFrag.x, worldPositionFrag.y, 0.0); // Project onto XY plane
            debugColor = vec4(1.0, 0.0, 0.0, 1.0); // Red for XY projection
        } else if (p.x > p.y && p.x > p.z) {
            projectedPosition = vec3(worldPositionFrag.y, worldPositionFrag.z, 0.0); // Project onto YZ plane
            debugColor = vec4(0.0, 1.0, 0.0, 1.0); // Green for YZ projection
        } else {
            projectedPosition = vec3(worldPositionFrag.x, worldPositionFrag.z, 0.0); // Project onto XZ plane
            debugColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue for XZ projection
        }
        gl_Position = vec4(projectedPosition, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}
