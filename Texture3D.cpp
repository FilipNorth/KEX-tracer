#include "Texture3D.h"

#include <iostream>

Texture3D::Texture3D(GLuint size_x, GLuint size_y, GLuint size_z,
    GLuint internal_format, GLuint mipmaps) {
    internal_format_ = internal_format;
    glGenTextures(1, &buffer_);
    glBindTexture(GL_TEXTURE_3D, buffer_);

    glTexStorage3D(GL_TEXTURE_3D, mipmaps + 1, internal_format, size_x, size_y, size_z);

    glBindTexture(GL_TEXTURE_3D, 0);
}
Texture3D::~Texture3D() {

}

void Texture3D::clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    clear_data[0] = r;
    clear_data[1] = g;
    clear_data[2] = b;
    clear_data[3] = a;
    clear();
}

void Texture3D::clear() {
    bind();

    GLuint format;
    GLuint type;
    if (internal_format_ == GL_R32UI) {
        format = GL_RED_INTEGER;
        type = GL_UNSIGNED_BYTE;
    }
    else {
        format = GL_RGBA;
        type = GL_FLOAT;
    }

    glClearTexImage(buffer_, 0, format, type, &clear_data);
}

void Texture3D::bind() {
    glBindTexture(GL_TEXTURE_3D, buffer_);
}
void Texture3D::bind(GLuint layout) {
    glActiveTexture(GL_TEXTURE0 + layout);
    bind();
}

void Texture3D::bind_image_texture(GLuint image_unit) {
    glBindImageTexture(image_unit, buffer_, 0, GL_TRUE, 0, GL_WRITE_ONLY, internal_format_);
}


void Texture3D::unbind() {
    glBindTexture(GL_TEXTURE_3D, 0);
}


void Texture3D::set_wrap(GLuint s, GLuint t, GLuint r) {
    bind();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    unbind();
}

void Texture3D::set_min_mag(GLuint min, GLuint mag) {
    bind();

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, min);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, mag);

    unbind();
}


void Texture3D::gen_mipmaps() {
    bind();
    glGenerateMipmap(GL_TEXTURE_3D);
}


void Texture3D::bind_image_read(GLuint location, GLuint format) {
    glBindImageTexture(location, buffer_, 0, GL_TRUE, 0, GL_READ_ONLY, format);
}
void Texture3D::bind_image_write(GLuint location, GLuint format) {
    glBindImageTexture(location, buffer_, 0, GL_TRUE, 0, GL_WRITE_ONLY, format);
}
void Texture3D::bind_image(GLuint location, GLuint format) {
    glBindImageTexture(location, buffer_, 0, GL_TRUE, 0, GL_READ_WRITE, format);
}