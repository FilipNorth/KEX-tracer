#version 460 core
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout(location = 2) in vec3 aColor;
layout (location = 3) in vec2 texcoord;

uniform mat4 camMatrix;
uniform mat4 model;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;

out vec3 worldPositionGeom;
out vec3 normalGeom;

void main(){
    mat4 M = translation * rotation * scale;  // Composite Model Matrix
    worldPositionGeom = vec3(camMatrix * vec4(Position, 1));  // Using the provided model matrix directly
    normalGeom = normalize(mat3(transpose(inverse(camMatrix))) * Normal);  // Adjusted for the 'model' matrix
    gl_Position = camMatrix * vec4(worldPositionGeom, 1);  // Using camMatrix as the View Matrix
}
