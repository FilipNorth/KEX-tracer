#version 460 core
layout(binding = 0, rgba8) uniform image3D voxelTexture;
uniform ivec3 gridSize;
// Lighting settings.
#define POINT_LIGHT_INTENSITY 1
#define MAX_LIGHTS 1

// Lighting attenuation factors.
#define DIST_FACTOR 1.1f /* Distance is multiplied by this when calculating attenuation. */
#define CONSTANT 1
#define LINEAR 0
#define QUADRATIC 1

// Returns an attenuation factor given a distance.
float attenuate(float dist){ dist *= DIST_FACTOR; return 1.0f / (CONSTANT + LINEAR * dist + QUADRATIC * dist * dist); }

layout(RGBA8) uniform image3D texture3D;

// Gets the Texture Units from the main function
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normalMap0;
uniform sampler2D metallicRoughnessMap0; 
// Assuming a combined texture for simplicity
// Gets the color of the light from the main function
uniform vec4 lightColor;
// Gets the position of the light from the main function
uniform vec3 lightPos;
// Gets the position of the camera from the main function
uniform vec3 camPos;

in vec3 worldPositionFrag;
in vec3 normalFrag;
in vec2 tex;
in vec3 crntPosFrag;

vec4 diffuse_coord = texture(diffuse0, tex);

vec3 calculatePointLight(){
	const vec3 direction = normalize(lightPos - worldPositionFrag);
	const float distanceToLight = distance(lightPos, worldPositionFrag);
	const float attenuation = attenuate(distanceToLight);
	const float d = max(dot(normalize(-normalFrag), direction), 0.0f);
	return d * POINT_LIGHT_INTENSITY * attenuation * lightColor.rgb;
}

vec4 pointLight()
{	
	// used in two variables so I calculate it here to not have to do it twice
	vec3 lightVec = lightPos - worldPositionFrag;

	// intensity of light with respect to distance
	float dist = length(lightVec);
	float a = 0.01; //og:3.0
	float b = 0.007; //og:0.7
	float inten = 3.0f / (a * dist * dist + b * dist + 1.0f);

	// ambient lighting
	float ambient = 0.20f;

	// diffuse lighting
	vec3 normal = normalize(normalFrag);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - worldPositionFrag);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	return (texture(diffuse0, tex) * (diffuse * inten + ambient) + texture(specular0, tex).r * specular * inten) * lightColor;
}

vec3 scaleAndBias(vec3 p) { return 0.5f * p + vec3(0.5f); }

bool isInsideCube(const vec3 p, float e) 
{ return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e; }

void main(){
	vec4 color = vec4(0.0f, 0, 0, 1);
	bool inside = isInsideCube(worldPositionFrag, 0);
	if(!inside) return;

	// Calculate diffuse lighting fragment contribution.
	color += pointLight();
	vec3 spec = vec3(texture(diffuse0, worldPositionFrag.xy));
	vec3 diff = vec3(texture(diffuse0, worldPositionFrag.xy));
	color.rgb = diffuse_coord.xyz * 0.1 + diffuse_coord.xyz * color.rgb; //* color + spec * 0.5f + diff * 0.5f;

 // Adjust worldPositionFrag to be within the texture's range before storing.
    vec3 voxel = scaleAndBias(worldPositionFrag);
    ivec3 voxelCoord = ivec3(vec3(voxel.x, voxel.y, voxel.z) * vec3(imageSize(texture3D))); // Correct scaling to voxel indices.
    imageStore(texture3D, voxelCoord, vec4(color)); // Store the computed color and alpha.
}