#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/shader_utils.hpp"

#include <GL/glew.h>

namespace datagui {

const static std::string vertex_shader = R"(
#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

out vec2 fs_uv;

void main(){
  gl_Position = vec4(pos.x, pos.y, 0, 1);
  fs_uv = uv;
}
)";

const static std::string fragment_shader = R"(
#version 330 core

in vec2 fs_uv;

uniform sampler2D tex;
out vec4 color;

void main(){
  color = vec4(1, 1, 1, texture(tex, fs_uv).x);
}
)";

Viewport::Viewport() :
    width(0),
    height(0),
    texture(0),
    framebuffer(0),
    program_id(0),
    VAO(0),
    VBO(0) {}

Viewport::~Viewport() {
  if (texture > 0) {
    glDeleteTextures(1, &texture);
  }
  if (framebuffer > 0) {
    glDeleteFramebuffers(1, &framebuffer);
  }
  if (program_id > 0) {
    glDeleteProgram(program_id);
  }
  if (VAO > 0) {
    glDeleteVertexArrays(1, &VAO);
  }
  if (VBO > 0) {
    glDeleteBuffers(1, &VBO);
  }
}

void Viewport::init(
    std::size_t width,
    std::size_t height,
    const std::shared_ptr<FontManager>& fm) {
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

  // Create a shader program for drawing the framebuffer texture

  program_id = create_program(vertex_shader, fragment_shader);
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glVertexAttribPointer(
      0,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)offsetof(Vertex, pos));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(
      1,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)offsetof(Vertex, uv));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

  // Initialise child class

  impl_init(fm);
}

void Viewport::update() {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  impl_render();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::render(const Box2& bounds) {
  glUseProgram(program_id);
  glBindVertexArray(VAO);

  std::vector<Vertex> vertices = {
      Vertex{bounds.lower_left(), Vec2(0, 1)},
      Vertex{bounds.lower_right(), Vec2(1, 1)},
      Vertex{bounds.upper_left(), Vec2(0, 0)},
      Vertex{bounds.lower_right(), Vec2(1, 1)},
      Vertex{bounds.upper_right(), Vec2(1, 0)},
      Vertex{bounds.upper_left(), Vec2(0, 0)}};

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      vertices.size() * sizeof(Vertex),
      vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindTexture(GL_TEXTURE_2D, texture);
  glDrawArrays(GL_TRIANGLES, 0, vertices.size());
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindVertexArray(0);
  glUseProgram(0);
}

} // namespace datagui
