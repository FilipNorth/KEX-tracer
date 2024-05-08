#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 camMatrix;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;


out vec3 worldPositionFrag;
out vec3 normalFrag;
out vec2 texCoords;

void main() {
    vec3 normalized = normalize(aPos);
    texCoords = aTexCoords;
    worldPositionFrag = vec3( model * vec4(aPos, 1));
    normalFrag = aNormal;
    gl_Position = vec4(worldPositionFrag, 1.0);
}