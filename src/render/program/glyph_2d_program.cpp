#include "datagui/render/program/glyph_2d_program.hpp"
#include "datagui/render/shaders.hpp"
#include <GL/glew.h>
#include <assert.h>

namespace dgui {

void Glyph2dProgram::init() {
  program_id = compile_program_vf(shaders::glyph_2d_vs, shaders::glyph_2d_fs);

  uniform_PV = glGetUniformLocation(program_id, "PV");
  uniform_text_color = glGetUniformLocation(program_id, "text_color");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &quad_VBO);
  glGenBuffers(1, &instance_VBO);

  struct QuadVertex {
    Vec2 pos;
  };

  // Configure static and instance arrays

  glBindVertexArray(VAO);
  {
    GLuint index = 0;

    // Assign static array attributes
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

    // Assign instance array attributes
    glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);

    for (std::size_t i = 0; i < 3; i++) {
      glVertexAttribPointer(
          index,
          3,
          GL_FLOAT,
          GL_FALSE,
          sizeof(Glyph2dInstance),
          (void*)(offsetof(Glyph2dInstance, M) + sizeof(float) * 3 * i));
      glVertexAttribDivisor(index, 1);
      glEnableVertexAttribArray(index);
      index++;
    }

    glVertexAttribPointer(
        index,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Glyph2dInstance),
        (void*)offsetof(Glyph2dInstance, uv_offset));
    glVertexAttribDivisor(index, 1);
    glEnableVertexAttribArray(index);
    index++;

    glVertexAttribPointer(
        index,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Glyph2dInstance),
        (void*)offsetof(Glyph2dInstance, uv_size));
    glVertexAttribDivisor(index, 1);
    glEnableVertexAttribArray(index);
    index++;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  glBindVertexArray(0);

  // Write static data to quad vertex buffer

  const std::vector<QuadVertex> quad_vertices = {
      {Vec2(0.f, 0.f)},
      {Vec2(1.f, 0.f)},
      {Vec2(0.f, 1.f)},
      {Vec2(1.f, 0.f)},
      {Vec2(1.f, 1.f)},
      {Vec2(0.f, 1.f)}};
  quad_vertex_count = quad_vertices.size();

  glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      quad_vertices.size() * sizeof(QuadVertex),
      quad_vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Glyph2dProgram::bind() {
  glUseProgram(program_id);
  glBindVertexArray(VAO);
}

void Glyph2dProgram::draw(
    unsigned int font_texture,
    const Color& color,
    const Glyph2dInstance* data,
    size_t count,
    const Mat3& PV) {

  // Stream data to instance buffer
  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      count * sizeof(Glyph2dInstance),
      data,
      GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUniformMatrix3fv(uniform_PV, 1, GL_FALSE, PV.data);
  glUniform4f(uniform_text_color, color.r, color.g, color.b, color.a);

  glBindTexture(GL_TEXTURE_2D, font_texture);
  glDrawArraysInstanced(GL_TRIANGLES, 0, quad_vertex_count, count);
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace dgui
