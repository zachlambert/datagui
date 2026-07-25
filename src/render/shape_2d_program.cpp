#include "datagui/render/shape_2d_program.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <assert.h>
#include <string>

namespace dgui {

const static std::string rect_vs = R"(
#version 330 core

layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec3 M_col1;
layout(location = 2) in vec3 M_col2;
layout(location = 3) in vec3 M_col3;
layout(location = 4) in vec4 color;
layout(location = 5) in vec4 border_color;
layout(location = 6) in vec2 border_width;
layout(location = 7) in vec2 radius;

uniform mat3 PV;

out vec2 fs_position_ms;

flat out vec4 fs_color;
flat out vec4 fs_border_color;
flat out vec2 fs_border_width;
flat out vec2 fs_radius;

void main() {
  mat3 M = mat3(M_col1, M_col2, M_col3);
  vec3 coords = PV * M * vec3(vertex_pos, 1);
  gl_Position = vec4(coords.xy / coords.z, 0, 1);

  fs_position_ms = vertex_pos;
  fs_color = color;
  fs_border_color = border_color;
  fs_border_width = border_width;
  fs_radius = radius;
}
)";

const static std::string rect_fs = R"(
#version 330 core

in vec2 fs_position_ms;
flat in vec4 fs_color;
flat in vec4 fs_border_color;
flat in vec2 fs_border_width;
flat in vec2 fs_radius;

out vec4 color;

void main() {
  vec2 size = vec2(0.5, 0.5);
  vec2 pos = abs(fs_position_ms);
  vec2 arc_origin = size - fs_radius;
  vec2 arc_pos = pos - arc_origin;

  if (fs_radius.x > 0 && fs_radius.y > 0
      && arc_pos.x >= 0 && arc_pos.y >= 0)
  {
    vec2 arc_pos_outer = arc_pos / fs_radius;
    vec2 arc_pos_inner = arc_pos / (fs_radius - fs_border_width);

    if (length(arc_pos_outer) > 1) {
      discard;
    } else if (length(arc_pos_inner) > 1) {
      color = fs_border_color;
    } else {
      color = fs_color;
    }
  } else {
    if (pos.x > size.x || pos.y > size.y) {
      discard;
    }
    if (pos.x > size.x - fs_border_width.x
        || pos.y > size.y - fs_border_width.y) {
      color = fs_border_color;
    } else {
      color = fs_color;
    }
  }
}
)";

void Shape2dProgram::init() {
  program_id = create_program(rect_vs, rect_fs);

  uniform_PV = glGetUniformLocation(program_id, "PV");

  // Generate ids
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &static_VBO);
  glGenBuffers(1, &instance_VBO);

  // Bind vertex array
  glBindVertexArray(VAO);

  struct Vertex {
    Vec2 position;
  };
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

  GLuint index = 0;

  glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Bind and configure buffer for indices
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

void Shape2dProgram::draw(
    const Shape2dInstance* data,
    size_t count,
    const Mat3& PV) {
  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      count * sizeof(Shape2dInstance),
      data,
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUseProgram(program_id);
  glUniformMatrix3fv(uniform_PV, 1, GL_FALSE, PV.data);
  glBindVertexArray(VAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, static_vertex_count, count);
}

} // namespace dgui
