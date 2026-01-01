#include "datagui/asset/image.hpp"
#include <GL/glew.h>

namespace datagui {

Image::Data::~Data() {
  if (texture > 0) {
    glDeleteTextures(1, &texture);
  }
}

Image::Data::Data(Data&& other) {
  texture = other.texture;
  other.texture = 0;
}

Image::Data& Image::Data::operator=(Data&& other) {
  if (texture > 0) {
    glDeleteTextures(1, &texture);
  }
  texture = other.texture;
  other.texture = 0;
  return *this;
}

void Image::load(std::size_t width, std::size_t height, void* pixels) {
  if (!data) {
    data = std::make_shared<Data>();
    glGenTextures(1, &data->texture);
  }
  data->width = width;
  data->height = height;

  glBindTexture(GL_TEXTURE_2D, data->texture);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      width,
      height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace datagui
