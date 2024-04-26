#version 460 core

#define INV_STEP_LENGTH (1.0f/STEP_LENGTH)
#define STEP_LENGTH 0.005f

uniform sampler2D textureBack; // Unit cube back FBO.
uniform sampler2D textureFront; // Unit cube front FBO.
uniform sampler3D texture3D; // Texture in which voxelization is stored.
uniform vec3 camPos; // World camera position.
uniform int state = 0; // Decides mipmap sample level.

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normalMap0;
uniform sampler2D metallicRoughnessMap0; 

in vec2 textureCoordinateFrag; 
out vec4 FragColor;

// Scales and bias a given vector (i.e. from [-1, 1] to [0, 1]).
vec3 scaleAndBias(vec3 p) { return 0.5f * p + vec3(0.5f); }

// Returns true if p is inside the unity cube (+ e) centered on (0, 0, 0).
bool isInsideCube(vec3 p, float e) { return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e; }

void main() {
	//Create basic test square


	const float mipmapLevel = state;

	// Initialize ray.
	const vec3 origin = isInsideCube(camPos, 0.2f) ? 
		camPos : texture(diffuse0, textureCoordinateFrag).xyz;
	vec3 direction = texture(diffuse0, textureCoordinateFrag).xyz - origin;
	const uint numberOfSteps = uint(INV_STEP_LENGTH * length(direction));
	direction = normalize(direction);

	// Trace.
	FragColor = vec4(0.89, 0.0, 0.0, 0.0);
	for(uint step = 0; step < numberOfSteps && FragColor.a < 0.99f; ++step) {
		const vec3 currentPoint = origin + STEP_LENGTH * step * direction;
		vec3 coordinate = scaleAndBias(currentPoint);
		vec4 currentSample = textureLod(texture3D, scaleAndBias(currentPoint), mipmapLevel);
		FragColor += (1.0f - FragColor.a) * currentSample;
	} 
	//FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / 2.2));
	FragColor.rgb = vec3(0.0, 1.0, 0.25);
}