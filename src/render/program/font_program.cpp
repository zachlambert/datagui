#include "datagui/render/program/font_program.hpp"
#include "datagui/render/shaders.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

namespace dgui {

namespace {
struct Vertex {
  Vec2 pos;
  Vec2 uv;
};
} // namespace

void FontProgram::init() {
  program_id = compile_program_vf(shaders::font_vs, shaders::font_fs);

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

void FontProgram::bind() {
  glUseProgram(program_id);
  glBindVertexArray(VAO);
}

void FontProgram::draw_bitmap(
    const Box2& box,
    int bitmap_width,
    int bitmap_height,
    const void* bitmap_data) {

  std::vector<Vertex> vertices = {
      Vertex{box.lower_left(), Vec2(0, 1)},
      Vertex{box.lower_right(), Vec2(1, 1)},
      Vertex{box.upper_left(), Vec2(0, 0)},
      Vertex{box.lower_right(), Vec2(1, 1)},
      Vertex{box.upper_right(), Vec2(1, 0)},
      Vertex{box.upper_left(), Vec2(0, 0)}};

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      vertices.size() * sizeof(Vertex),
      vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate a texture for the character bitmap
  unsigned int char_texture;
  glGenTextures(1, &char_texture);
  glBindTexture(GL_TEXTURE_2D, char_texture);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RED,
      bitmap_width,
      bitmap_height,
      0,
      GL_RED,
      GL_UNSIGNED_BYTE,
      bitmap_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glDrawArrays(GL_TRIANGLES, 0, vertices.size());

  glBindTexture(GL_TEXTURE_2D, 0);
  glDeleteTextures(1, &char_texture);
}

} // namespace dgui
