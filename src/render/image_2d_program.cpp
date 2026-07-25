#include "datagui/render/image_2d_program.hpp"
#include "datagui/render/embedded_shaders.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <string>

namespace dgui {

void Image2dProgram::init() {
  // Configure shader program and buffers

  program_id = create_program(shaders::image_2d_vs, shaders::image_2d_fs);
  uniform_PV = glGetUniformLocation(program_id, "PV");

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
      sizeof(Image2dVertex),
      (void*)offsetof(Image2dVertex, pos));
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Image2dVertex),
      (void*)offsetof(Image2dVertex, uv));
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Image2dProgram::bind() {
  glUseProgram(program_id);
  glBindVertexArray(VAO);
}

void Image2dProgram::unbind() {
  glBindVertexArray(0);
  glUseProgram(0);
}

void Image2dProgram::draw(
    unsigned int texture,
    const Image2dVertex* data,
    size_t count,
    const Mat3& PV) {
  glUniformMatrix3fv(uniform_PV, 1, GL_FALSE, PV.data);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      count * sizeof(Image2dVertex),
      data,
      GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindTexture(GL_TEXTURE_2D, texture);
  glDrawArrays(GL_TRIANGLES, 0, count);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindVertexArray(0);
  glUseProgram(0);
}

} // namespace dgui
