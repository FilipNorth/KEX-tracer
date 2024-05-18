#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// Interpolated values from the vertex shaders
out vec2 UV;
out vec3 Position_world;
out vec3 Normal_world;
out vec3 Tangent_world;
out vec3 Bitangent_world;
out vec3 EyeDirection_world;
out vec4 Position_depth; // Position from the shadow map point of view
out flat int axis;
out mat4 projectionMatrix;
out vec4 Position_screen;


in vData {
    vec2 UV;
    vec3 Position_world;
    vec3 Normal_world;
    vec3 Tangent_world;
    vec3 Bitangent_world;
    vec3 EyeDirection_world;
    vec4 Position_depth;
    vec4 Position_screen;
} vertices[];


uniform mat4 ProjX;
uniform mat4 ProjY;
uniform mat4 ProjZ;

void main() {
    vec3 p1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 normal = normalize(cross(p1,p2));

    float nDotX = abs(normal.x);
    float nDotY = abs(normal.y);
    float nDotZ = abs(normal.z);

    // 0 = x axis dominant, 1 = y axis dominant, 2 = z axis dominant
    axis = (nDotX >= nDotY && nDotX >= nDotZ) ? 1 : (nDotY >= nDotX && nDotY >= nDotZ) ? 2 : 3;
    projectionMatrix = axis == 1 ? ProjX : axis == 2 ? ProjY : ProjZ;
    
    // For every vertex sent in vertices
    for(int i = 0;i < gl_in.length(); i++) {
        vec3 middlePos = gl_in[0].gl_Position.xyz / 3.0 + gl_in[1].gl_Position.xyz / 3.0 + gl_in[2].gl_Position.xyz / 3.0;
        UV = vertices[i].UV;
        Position_depth = vertices[i].Position_depth;
        //gl_Position = projectionMatrix * gl_in[i].gl_Position;
        gl_Position = gl_in[i].gl_Position;
        Position_world = vertices[i].Position_world;
        Normal_world = vertices[i].Normal_world;
        Tangent_world = vertices[i].Tangent_world;
        Bitangent_world = vertices[i].Bitangent_world;
        EyeDirection_world = vertices[i].EyeDirection_world;
        Position_screen = projectionMatrix * vertices[i].Position_screen;


        EmitVertex();
    }
    
    // Finished creating vertices
    EndPrimitive();
}

