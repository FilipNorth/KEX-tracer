#version 460 core

in vec3 normal_world;
in vec4 fragColor;

out vec4 color;

void main() {
	if(fragColor.a < 0.1f)
		discard;

	color = vec4(fragColor);
	//color = vec4(normal_world, 1.0);
}