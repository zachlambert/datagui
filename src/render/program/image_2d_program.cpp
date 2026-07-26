#include "datagui/render/program/image_2d_program.hpp"
#include "datagui/render/shaders.hpp"
#include <GL/glew.h>
#include <vector>

namespace dgui {

void Image2dProgram::init() {
  program_id = compile_program_vf(shaders::image_2d_vs, shaders::image_2d_fs);
  uniform_PVM = glGetUniformLocation(program_id, "PVM");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &static_VBO);

  struct Vertex {
    Vec2 pos;
    Vec2 uv;
  };

  // Configure vertex array

  glBindVertexArray(VAO);
  {
    GLuint index = 0;

    glBindBuffer(GL_ARRAY_BUFFER, static_VBO);

    glVertexAttribPointer(
        index,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(index);
    index++;

    glVertexAttribPointer(
        index,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(index);
    index++;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  glBindVertexArray(0);

  // Write static vertex buffer data

  const std::vector<Vertex> static_vertices = {
      {Vec2(0.f, 1.f), Vec2(0.f, 1.f)},
      {Vec2(1.f, 0.f), Vec2(1.f, 0.f)},
      {Vec2(0.f, 0.f), Vec2(0.f, 0.f)},
      {Vec2(0.f, 1.f), Vec2(0.f, 1.f)},
      {Vec2(1.f, 0.f), Vec2(1.f, 0.f)},
      {Vec2(1.f, 1.f), Vec2(1.f, 1.f)}};
  static_vertex_count = static_vertices.size();

  glBindBuffer(GL_ARRAY_BUFFER, static_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      static_vertices.size() * sizeof(Vertex),
      static_vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Image2dProgram::bind() {
  glUseProgram(program_id);
  glBindVertexArray(VAO);
}

void Image2dProgram::draw(unsigned int texture, const Mat3& M, const Mat3& PV) {
  Mat3 PVM = PV * M;
  std::cout << PVM << std::endl;
  glUniformMatrix3fv(uniform_PVM, 1, GL_FALSE, PVM.data);
  glBindTexture(GL_TEXTURE_2D, texture);
  glDrawArrays(GL_TRIANGLES, 0, static_vertex_count);
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace dgui
