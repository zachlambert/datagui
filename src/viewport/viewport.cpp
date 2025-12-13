#include "datagui/viewport/viewport.hpp"
#include <GL/glew.h>

namespace datagui {

Viewport::Viewport() : width(0), height(0), texture_(0), framebuffer(0) {}

Viewport::~Viewport() {
  if (texture_ > 0) {
    glDeleteTextures(1, &texture_);
  }
  if (framebuffer > 0) {
    glDeleteFramebuffers(1, &framebuffer);
  }
}

Viewport::Viewport(Viewport&& other) {
  width = other.width;
  height = other.width;
  texture_ = other.texture_;
  framebuffer = other.framebuffer;

  other.texture_ = 0;
  other.framebuffer = 0;
}

void Viewport::init(
    std::size_t width,
    std::size_t height,
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  this->width = width;
  this->height = height;

  // Create a texture to render to

  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);
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
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, 0);

  // Create render buffer
  glGenRenderbuffers(1, &render_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

  // Bind render buffer to frame buffer
  glFramebufferRenderbuffer(
      GL_FRAMEBUFFER,
      GL_DEPTH_STENCIL_ATTACHMENT,
      GL_RENDERBUFFER,
      render_buffer);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // Initialise child class

  impl_init(theme, fm);
}

void Viewport::bind_framebuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glViewport(0, 0, width, height);
  glClearColor(1.f, 1.f, 1.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Viewport::unbind_framebuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DEPTH_BUFFER, 0);
}

} // namespace datagui
