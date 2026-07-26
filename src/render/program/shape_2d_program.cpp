#include "datagui/render/program/shape_2d_program.hpp"
#include "datagui/render/shaders.hpp"
#include <GL/glew.h>
#include <assert.h>

namespace dgui {

void Shape2dProgram::init() {
  program_id = compile_program_vf(shaders::shape_2d_vs, shaders::shape_2d_fs);

  uniform_PV = glGetUniformLocation(program_id, "PV");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &static_VBO);
  glGenBuffers(1, &instance_VBO);

  struct Vertex {
    Vec2 pos;
  };

  // Configure static and instance arrays

  glBindVertexArray(VAO);
  {
    GLuint index = 0;

    // Assign static array attributes
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

    // Assign instance array attributes
    glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);

    for (std::size_t i = 0; i < 3; i++) {
      glVertexAttribPointer(
          index,
          3,
          GL_FLOAT,
          GL_FALSE,
          sizeof(Shape2dInstance),
          (void*)(offsetof(Shape2dInstance, M) + sizeof(float) * 3 * i));
      glVertexAttribDivisor(index, 1);
      glEnableVertexAttribArray(index);
      index++;
    }

    glVertexAttribPointer(
        index,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Shape2dInstance),
        (void*)offsetof(Shape2dInstance, color));
    glVertexAttribDivisor(index, 1);
    glEnableVertexAttribArray(index);
    index++;

    glVertexAttribPointer(
        index,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Shape2dInstance),
        (void*)offsetof(Shape2dInstance, border_color));
    glVertexAttribDivisor(index, 1);
    glEnableVertexAttribArray(index);
    index++;

    glVertexAttribPointer(
        index,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Shape2dInstance),
        (void*)offsetof(Shape2dInstance, border_width));
    glVertexAttribDivisor(index, 1);
    glEnableVertexAttribArray(index);
    index++;

    glVertexAttribPointer(
        index,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Shape2dInstance),
        (void*)offsetof(Shape2dInstance, radius));
    glVertexAttribDivisor(index, 1);
    glEnableVertexAttribArray(index);
    index++;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  glBindVertexArray(0);

  const std::vector<Vertex> static_vertices = {
      {Vec2(-0.5f, 0.5f)},
      {Vec2(0.5f, -0.5f)},
      {Vec2(-0.5f, -0.5f)},
      {Vec2(-0.5f, 0.5f)},
      {Vec2(0.5f, -0.5f)},
      {Vec2(0.5f, 0.5f)}};
  static_vertex_count = static_vertices.size();

  // Bind and configure buffer for vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, static_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      static_vertices.size() * sizeof(Vertex),
      static_vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Shape2dProgram::bind() {
  glUseProgram(program_id);
  glBindVertexArray(VAO);
}

void Shape2dProgram::draw(
    const Shape2dInstance* data,
    size_t count,
    const Mat3& PV) {

  // Stream data to instance buffer
  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      count * sizeof(Shape2dInstance),
      data,
      GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUniformMatrix3fv(uniform_PV, 1, GL_FALSE, PV.data);
  glDrawArraysInstanced(GL_TRIANGLES, 0, static_vertex_count, count);
}

} // namespace dgui
