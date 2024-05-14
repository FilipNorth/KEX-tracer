#version 460 core

/*
layout(location = 0) in vec3 vertexPosition_model;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_model;
layout(location = 3) in vec3 vertexTangent_model;
layout(location = 4) in vec3 vertexBitangent_model;
*/

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout(location = 2) in vec3 aColor;
layout (location = 3) in vec2 texcoord;

uniform mat4 DepthModelViewProjectionMatrix;
uniform mat4 ModelMatrix;

out vData {
    vec2 UV;
    vec4 position_depth;
} vert;

void main() {
    vert.UV = vec2(texcoord.y, -texcoord.x);
    vert.position_depth = DepthModelViewProjectionMatrix * vec4(Position, 1);
	vert.position_depth.xyz = vert.position_depth.xyz * 0.5f + 0.5f;

    gl_Position = ModelMatrix * vec4(Position,1);
}