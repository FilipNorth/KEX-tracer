
#include<glad/glad.h>
#include<stb/stb_image.h.txt>

#include"shaderClass.h"

class Texture
{
public:
	GLuint ID;
	const char* type;
	GLuint unit;
	int widthImg, heightImg, numColCh;

	Texture(const char* image, const char* texType, GLuint slot);

	// Assigns a texture unit to a texture
	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	// Binds a texture
	void Bind();
	// Unbinds a texture
	void Unbind();
	// Deletes a texture
	void Delete();


};

struct Texture2D {
	GLuint textureID;
	int width, height, componentsPerPixel;
};

struct Texture3D {
	GLuint textureID;
	int size, componentsPerPixel;
};