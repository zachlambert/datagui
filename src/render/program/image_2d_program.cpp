#include "datagui/render/program/image_2d_program.hpp"
#include "datagui/render/shaders.hpp"
#include <GL/glew.h>
#include <vector>

namespace dgui {

void Image2dProgram::init() {
  program_id = compile_program_vf(shaders::image_2d_vs, shaders::image_2d_fs);
  uniform_PVM = glGetUniformLocation(program_id, "PVM");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &quad_VBO);

  struct QuadVertex {
    Vec2 pos;
    Vec2 uv;
  };

  // Configure vertex array

  glBindVertexArray(VAO);
  {
    GLuint index = 0;

    glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);

    glVertexAttribPointer(
        index,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(QuadVertex),
        (void*)offsetof(QuadVertex, pos));
    glEnableVertexAttribArray(index);
    index++;

    glVertexAttribPointer(
        index,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(QuadVertex),
        (void*)offsetof(QuadVertex, uv));
    glEnableVertexAttribArray(index);
    index++;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  glBindVertexArray(0);

  // Write static vertex buffer data

  const std::vector<QuadVertex> quad_vertices = {
      {Vec2(0.f, 1.f), Vec2(0.f, 1.f)},
      {Vec2(1.f, 0.f), Vec2(1.f, 0.f)},
      {Vec2(0.f, 0.f), Vec2(0.f, 0.f)},
      {Vec2(0.f, 1.f), Vec2(0.f, 1.f)},
      {Vec2(1.f, 0.f), Vec2(1.f, 0.f)},
      {Vec2(1.f, 1.f), Vec2(1.f, 1.f)}};
  quad_vertex_count = quad_vertices.size();

  glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      quad_vertices.size() * sizeof(QuadVertex),
      quad_vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Image2dProgram::bind() {
  glUseProgram(program_id);
  glBindVertexArray(VAO);
}

void Image2dProgram::draw(
    const Mat3& PV,
    unsigned int texture,
    const Mat3& transform) {
  const Mat3 PVM = PV * transform;
  glUniformMatrix3fv(uniform_PVM, 1, GL_FALSE, PVM.data);
  glBindTexture(GL_TEXTURE_2D, texture);
  glDrawArrays(GL_TRIANGLES, 0, quad_vertex_count);
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace dgui
