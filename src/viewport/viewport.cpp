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
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Initialise child class

  impl_init(theme, fm);
}

void Viewport::render_content(const Color& fill_color) {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glViewport(0, 0, width, height);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(fill_color.r, fill_color.g, fill_color.b, 1.f);
  glClearDepth(0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  impl_render();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace datagui
