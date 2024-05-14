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

out vec2 UV;
out vec3 Position_world;
out vec3 Normal_world;
out vec3 Tangent_world;
out vec3 Bitangent_world;
out vec3 EyeDirection_world;
out vec3 LightDirection_tangent;
out vec3 EyeDirection_tangent;
out vec4 Position_depth;

uniform vec3 CameraPosition;
uniform vec3 LightDirection;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 DepthModelViewProjectionMatrix;

vec3 createOrthogonalVectors(vec3 n) {
    vec3 tangent;
    if (abs(n.x) > abs(n.z)) {
        tangent = vec3(-n.y, n.x, 0.0);
    } else {
        tangent = vec3(0.0, -n.z, n.y);
    }
    return tangent = normalize(tangent);
}
vec2 scaleAndBias(vec2 p) { return 0.5f * p + vec2(0.5f); }
void main() {
    vec3 tangent = createOrthogonalVectors(Normal); // Declare tangent (and bitangent
    vec3 bitangent = cross(Normal, tangent);

	gl_Position =  ProjectionMatrix * ModelViewMatrix * vec4(Position,1);

	Position_world = (ModelMatrix * vec4(Position,1)).xyz;

	Position_depth = DepthModelViewProjectionMatrix * vec4(Position, 1);
	Position_depth.xyz = Position_depth.xyz * 0.5 + 0.5;

	Normal_world = normalize((ModelMatrix * vec4(Normal,0)).xyz);
	Tangent_world = normalize((ModelMatrix * vec4(tangent,0)).xyz);
	Bitangent_world = normalize((ModelMatrix * vec4(bitangent,0)).xyz);

	EyeDirection_world = CameraPosition - Position_world; // Normalize in fragment shader or else it will be interpolated wrong

	UV = vec2(texcoord.y, -texcoord.x);
    //UV = texcoord;
}