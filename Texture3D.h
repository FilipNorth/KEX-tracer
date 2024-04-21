#pragma once

#include<glad/glad.h>

class Texture3D
{
	GLuint buffer_ = 0;
	GLuint internal_format_;
	GLfloat clear_data[4] = { 0,0,0,0 };

public:
	Texture3D() {};
	// Texture3D(GLuint size, GLuint internal_format, GLuint mipmaps = 1) : Texture3D(size,size,size, internal_format, mipmaps){};
	Texture3D(GLuint size_x, GLuint size_y, GLuint size_z,
		GLuint internal_format, GLuint mipmaps = 1);
	~Texture3D();

	void gen_mipmaps();

	void clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	void clear();


	void bind();
	void bind(GLuint layout);
	void bind_image_texture(GLuint image_unit);
	void unbind();

	void set_wrap(GLuint all) { set_wrap(all, all, all); };
	void set_wrap(GLuint s, GLuint t, GLuint r);
	void set_min_mag(GLuint min, GLuint mag);

	void bind_image_read(GLuint location, GLuint format);
	void bind_image_write(GLuint location, GLuint format);
	void bind_image(GLuint location, GLuint format);
};

