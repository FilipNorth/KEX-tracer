#version 460 core

uniform mat4 V;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout(location = 2) in vec3 aColor;
layout (location = 3) in vec2 texcoord;

out vec2 textureCoordinateFrag;
out vec3 PositionFrag;

// Scales and bias a given vector (i.e. from [-1, 1] to [0, 1]).
vec2 scaleAndBias(vec2 p) { return 0.5f * p + vec2(0.5f); }

void main(){
	textureCoordinateFrag = scaleAndBias(Position.xy);
	PositionFrag = Position;
	gl_Position = vec4(Position, 1);
}
