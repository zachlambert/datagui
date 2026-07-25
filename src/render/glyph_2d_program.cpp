#include "datagui/render/glyph_2d_program.hpp"
#include "datagui/render/embedded_shaders.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <assert.h>
#include <string>

namespace dgui {

void Glyph2dProgram::init() {
  // Configure shader program and buffers

  program_id = create_program(shaders::glyph_2d_vs, shaders::glyph_2d_fs);
  uniform_PV = glGetUniformLocation(program_id, "PV");
  uniform_text_color = glGetUniformLocation(program_id, "text_color");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  GLuint index = 0;

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Glyph2dVertex),
      (void*)offsetof(Glyph2dVertex, pos));
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Glyph2dVertex),
      (void*)offsetof(Glyph2dVertex, uv));
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Glyph2dProgram::bind() {
  glUseProgram(program_id);
  glBindVertexArray(VAO);
}

void Glyph2dProgram::unbind() {
  glBindVertexArray(0);
  glUseProgram(0);
}

void Glyph2dProgram::draw(
    unsigned int font_texture,
    const Color& color,
    const Glyph2dVertex* data,
    size_t count,
    const Mat3& PV) {
  glUniformMatrix3fv(uniform_PV, 1, GL_FALSE, PV.data);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      count * sizeof(Glyph2dVertex),
      data,
      GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUniform4f(uniform_text_color, color.r, color.g, color.b, color.a);

  glBindTexture(GL_TEXTURE_2D, font_texture);
  glDrawArrays(GL_TRIANGLES, 0, count);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindVertexArray(0);
  glUseProgram(0);
}

} // namespace dgui
