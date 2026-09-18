#include "datagui/render/program/shape_3d_program.hpp"
#include "datagui/render/shaders.hpp"
#include "datagui/render/state/shape_3d_vertex.hpp"
#include <GL/glew.h>
#include <cmath>

namespace dgui {

void Shape3dProgram::init() {
  program_id = compile_program_vf(shaders::shape_3d_vs, shaders::shape_3d_fs);

  uniform_P = glGetUniformLocation(program_id, "P");
  uniform_V = glGetUniformLocation(program_id, "V");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &static_VBO);
  glGenBuffers(1, &static_EBO);
  glGenBuffers(1, &instance_VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_EBO);

  {
    // Bind and configure buffer for vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, static_VBO);

    GLuint index = 0;

    glVertexAttribPointer(
        index,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Shape3dVertex),
        (void*)(offsetof(Shape3dVertex, position)));
    glEnableVertexAttribArray(index);
    index++;

    glVertexAttribPointer(
        index,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Shape3dVertex),
        (void*)(offsetof(Shape3dVertex, normal)));
    glEnableVertexAttribArray(index);
    index++;

    // Bind and configure buffer for instance attributes
    glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);

    for (std::size_t i = 0; i < 4; i++) {
      // Column i of the transform
      glVertexAttribPointer(
          index,
          4,
          GL_FLOAT,
          GL_FALSE,
          sizeof(Shape3dInstance),
          (void*)(offsetof(Shape3dInstance, transform) +
                  sizeof(float) * 4 * i));
      glVertexAttribDivisor(index, 1);
      glEnableVertexAttribArray(index);
      index++;
    }

    glVertexAttribPointer(
        index,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Shape3dInstance),
        (void*)offsetof(Shape3dInstance, color));
    glVertexAttribDivisor(index, 1);
    glEnableVertexAttribArray(index);
    index++;
  }
  glBindVertexArray(0);

  // Allocate buffer data

  shapes.resize(Shape3dTypeCount);

  std::vector<Shape3dVertex> vertices;
  std::vector<unsigned int> indices;

  for (size_t i = 0; i < Shape3dTypeCount; i++) {
    auto& shape = shapes[i];
    shape.indices_begin = indices.size();
    add_shape_vertices((Shape3dType)i, vertices, indices);
    shape.indices_end = indices.size();
  }

  glBindBuffer(GL_ARRAY_BUFFER, static_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      vertices.size() * sizeof(Shape3dVertex),
      vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_EBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      indices.size() * sizeof(unsigned int),
      indices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Shape3dProgram::bind() {
  glUseProgram(program_id);
  glBindVertexArray(VAO);
}

void Shape3dProgram::draw(
    const Mat4& view,
    const Mat4& projection,
    Shape3dType type,
    const Shape3dInstance* data,
    size_t count) {
  if (count == 0) {
    return;
  }
  const auto& shape = shapes[(std::size_t)type];

  glUniformMatrix4fv(uniform_P, 1, GL_FALSE, projection.data);
  glUniformMatrix4fv(uniform_V, 1, GL_FALSE, view.data);

  // Stream data to instance buffer
  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      count * sizeof(Shape3dInstance),
      data,
      GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawElementsInstanced(
      GL_TRIANGLES,
      (shape.indices_end - shape.indices_begin),
      GL_UNSIGNED_INT,
      (void*)(sizeof(unsigned int) * shape.indices_begin),
      count);
}

} // namespace dgui
