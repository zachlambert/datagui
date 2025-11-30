#include "datagui/visual/image_shader.hpp"
#include "datagui/geometry/rot.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <vector>

namespace datagui {

OpenglRgbImage::~OpenglRgbImage() {
  if (texture_ > 0) {
    glDeleteTextures(1, &texture_);
  }
}

OpenglRgbImage::OpenglRgbImage(OpenglRgbImage&& other) {
  texture_ = other.texture_;
  other.texture_ = 0;
}

void OpenglRgbImage::write(
    std::size_t width,
    std::size_t height,
    void* pixels) {
  if (texture_ == 0) {
    glGenTextures(1, &texture_);
  }
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
      pixels);
  glBindTexture(GL_TEXTURE_2D, 0);
}

const static std::string vertex_shader = R"(
#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

uniform vec2 viewport_size;
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
  color = texture(tex, fs_uv);
}
)";

ImageShader::~ImageShader() {
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

ImageShader::ImageShader(ImageShader&& other) {
  program_id = other.program_id;
  uniform_viewport_size = other.uniform_viewport_size;
  VAO = other.VAO;
  VBO = other.VBO;

  other.program_id = 0;
  other.VAO = 0;
  other.VBO = 0;
}

void ImageShader::init() {
  program_id = create_program(vertex_shader, fragment_shader);
  uniform_viewport_size = glGetUniformLocation(program_id, "viewport_size");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

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

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void ImageShader::draw(int texture, const Box2& box) {
  draw(texture, box.lower, 0, box.size());
}

void ImageShader::draw(
    int texture,
    const Vec2& position,
    double angle,
    const Vec2& size) {

  Mat2 R = Rot2(angle).mat();
  Vec2 lower_left = position;
  Vec2 lower_right = position + R * Vec2(size.x, 0);
  Vec2 upper_left = position + R * Vec2(0, size.y);
  Vec2 upper_right = position + R * size;

  std::vector<Vertex> vertices = {
      Vertex{lower_left, Vec2(0, 1)},
      Vertex{lower_right, Vec2(1, 1)},
      Vertex{upper_left, Vec2(0, 0)},
      Vertex{lower_right, Vec2(1, 1)},
      Vertex{upper_right, Vec2(1, 0)},
      Vertex{upper_left, Vec2(0, 0)}};

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      vertices.size() * sizeof(Vertex),
      vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUseProgram(program_id);
  glBindVertexArray(VAO);
  glBindTexture(GL_TEXTURE_2D, texture);

  glDrawArrays(GL_TRIANGLES, 0, vertices.size());

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

} // namespace datagui
