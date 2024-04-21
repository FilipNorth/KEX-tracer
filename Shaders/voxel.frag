#version 460 core

// Outputs colors in RGBA
out vec4 FragColor;

// Imports the current position from the Vertex Shader
in vec3 crntPos;
// Imports the normal from the Vertex Shader
in vec3 Normal;
// Imports the color from the Vertex Shader
in vec3 color;
// Imports the texture coordinates from the Vertex Shader
in vec2 texCoord;

struct PointLight_t {
  vec3 position; //offset 0
  vec3 color; //offset 16
  float intensity; //offset 32
};

struct DirectionalLight_t {
  vec3 direction; //offset 0
  vec3 color; //offset 16
  float intensity; //offset 32
};

layout (rgba8) uniform coherent volatile image3D voxel_field;
//layout (rgba8) uniform coherent volatile image3D voxel_field;
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

vec4 albedo = texture(diffuse0, texCoord);
vec4 specular = texture(specular0, texCoord);
vec4 normal = texture(normalMap0, texCoord);
vec4 metallicRoughness = texture(metallicRoughnessMap0, texCoord);


vec4 pointLight()
{	
	// used in two variables so I calculate it here to not have to do it twice
	vec3 lightVec = lightPos - crntPos;

	// intensity of light with respect to distance
	float dist = length(lightVec);
	float a = 0.01; //og:3.0
	float b = 0.007; //og:0.7
	float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

	// ambient lighting
	float ambient = 0.20f;

	// diffuse lighting
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	return (texture(diffuse0, texCoord) * (diffuse * inten + ambient) + texture(specular0, texCoord).r * specular * inten) * lightColor;
}




vec3 calculate_light(vec3 light_dir, vec3 light_color, float light_intensity,
		     bool attenuation) {
  float light_distance = length(light_dir);

  float NdotL = dot(Normal, normalize(light_dir));
  float intensity = max(NdotL, 0.);
  vec3 color = albedo.xyz * intensity * light_color * light_intensity;

  if (attenuation)
    color /= light_distance + 1;

  return color;
}

vec3 shade() {
  vec3 color = vec3(0);

  const int DirectionalLightCount = 1;
  const int PointLightCount = 1;

  DirectionalLight_t DirectionalLights[DirectionalLightCount];
  DirectionalLights[0] = DirectionalLight_t(vec3(.2, -1, 0),
					    vec3(1.),
					    1.);


  PointLight_t PointLights[PointLightCount];
  PointLights[0] = PointLight_t(vec3(0, 2, 0), vec3(1), 10.);

  for (int i = 0; i < PointLightCount; i++) {
    PointLight_t ls = PointLights[i];
    vec3 light_dir = ls.position - crntPos;
    color += calculate_light(light_dir, ls.color, ls.intensity, true);
  }

  return color;
  for (int i = 0; i < DirectionalLightCount; i++) {
    DirectionalLight_t ls = DirectionalLights[i];
    vec3 light_dir = -ls.direction;
    color += calculate_light(light_dir, ls.color, ls.intensity, false);
  }

  return color;
}

void main(){
  ivec3 image_size = imageSize(voxel_field);

  vec4 res  = vec4(shade(), 1.0);

  //float shadow = shadow_calculation(lightPos);
  //res.rgb *= 0.5 * (1.0 - shadow) + 0.5;
  res.rgb = pow(res.rgb, vec3(1.0 / 2.2));

  imageStore(voxel_field,
	     ivec3(image_size * (0.5 * crntPos + 0.5)), res);
}