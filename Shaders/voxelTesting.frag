#version 460 core
// Lighting settings.
out vec4 FragColor;
#define POINT_LIGHT_INTENSITY 1
#define MAX_LIGHTS 1

// Lighting attenuation factors.
#define DIST_FACTOR 1.1f /* Distance is multiplied by this when calculating attenuation. */
#define CONSTANT 1
#define LINEAR 0
#define QUADRATIC 1

// Returns an attenuation factor given a distance.
float attenuate(float dist){ dist *= DIST_FACTOR; return 1.0f / (CONSTANT + LINEAR * dist + QUADRATIC * dist * dist); }



// Gets the Texture Units from the main function
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normalMap0;
uniform sampler2D metallicRoughnessMap0; 

// Gets the color of the light from the main function
uniform vec3 lightColor2;
// Gets the position of the light from the main function
uniform vec3 lightPos;
// Gets the position of the camera from the main function
uniform vec3 camPos;

layout(RGBA8) uniform image3D texture3D;

in vec3 worldPositionFrag;
in vec3 normalFrag;

vec3 calculatePointLight(){
	const vec3 direction = normalize(lightPos - worldPositionFrag);
	const float distanceToLight = distance(lightPos, worldPositionFrag);
	const float attenuation = attenuate(distanceToLight);
	const float d = max(dot(normalize(normalFrag), direction), 0.0f);
	return d * POINT_LIGHT_INTENSITY * attenuation * lightColor2;
}

vec3 scaleAndBias(vec3 p) { return 0.5f * p + vec3(0.5f); }

bool isInsideCube(const vec3 p, float e) { return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e; }

void main(){
	vec3 color = vec3(0.26, 0.53, 0.36);
	if(!isInsideCube(worldPositionFrag, 0)) return;

	// Calculate diffuse lighting fragment contribution.
	const uint maxLights = 1;
	for(uint i = 0; i < maxLights; ++i) color += calculatePointLight();
vec3 spec = texture(specular0, vec2(1.0)).rgb;
vec3 diff = texture(diffuse0, vec2(1.0)).rgb;
	color = (diff + spec) * color;

	// Output lighting to 3D texture.
	vec3 voxel = scaleAndBias(worldPositionFrag);
	ivec3 dim = imageSize(texture3D);
	float alpha = pow(1, 4); // For soft shadows to work better with transparent materials.
	vec4 res = alpha * vec4(vec3(color), 1);
    imageStore(texture3D, ivec3(dim * voxel), res);
    FragColor = vec4(color, 1);
}
