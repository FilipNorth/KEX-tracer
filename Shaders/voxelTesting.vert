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


out vec3 worldPositionGeom;
out vec3 normalGeom;
out vec2 texCoords;
out vec3 crntPos;
out mat4 projection;

void main() {
    vec3 normalized = normalize(aPos);
    texCoords = aTexCoords;
    worldPositionGeom = vec3( model * vec4(aPos, 1));
    normalGeom = aNormal;
    gl_Position = vec4(worldPositionGeom, 1.0);
    crntPos = vec3(model * translation * rotation * scale * vec4(aPos, 1.0f));
    projection = camMatrix;
}