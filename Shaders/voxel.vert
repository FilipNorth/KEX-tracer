#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTexCoords;

out DATA{
    vec3 Normal;
    vec3 color;
    vec2 texCoord;
    mat4 projection;
    vec3 crntPos;
} data_out;


uniform mat4 camMatrix;
uniform mat4 model;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;

void main(){
	vec3 crntPos = vec3(camMatrix * vec4(aPos, 1));
	vec3 Normal = normalize(mat3(transpose(inverse(camMatrix))) * aNormal);
	//gl_Position = P * V * vec4(crntPos, 1);
}
