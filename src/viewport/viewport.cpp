#include "datagui/viewport/viewport.hpp"

#include <GL/glew.h>

namespace datagui {

Viewport::Viewport() : width(0), height(0), texture(0), framebuffer(0) {}

Viewport::~Viewport() {
  if (texture > 0) {
    glDeleteTextures(1, &texture);
  }
  if (framebuffer > 0) {
    glDeleteFramebuffers(1, &framebuffer);
  }
}

void Viewport::init(std::size_t width, std::size_t height) {
  this->width = width;
  this->height = height;

  // Create a texture to render to

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB,
      width,
      height,
      0,
      GL_RGB,
      GL_UNSIGNED_BYTE,
      0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Create a framebuffer and bind a texture to it

  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
}

void Viewport::bind_framebuffer() const {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void Viewport::unbind_framebuffer() const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace datagui
