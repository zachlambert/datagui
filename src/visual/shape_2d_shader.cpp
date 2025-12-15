#include "datagui/visual/shape_2d_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <assert.h>
#include <string>

namespace datagui {

const static std::string rect_vs = R"(
#version 330 core

layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec3 M_col1;
layout(location = 2) in vec3 M_col2;
layout(location = 3) in vec3 M_col3;
layout(location = 4) in vec2 radius;
layout(location = 5) in vec2 border_width;
layout(location = 6) in vec4 color;
layout(location = 7) in vec4 border_color;

uniform Mat3 PV;

out vec2 fs_position_ms;
flat out vec2 fs_radius;
flat out vec2 fs_border_width;
flat out vec4 fs_color;
flat out vec4 fs_border_color

void main(){
  mat3 M = mat3(M_col1, M_col2, M_col3);
  gl_Position = vec4((PV * M * vec3(vertex_pos, 1)).xy, 0, 1);

  fs_position_ms = vertex_pos;
  fs_radius = radius;
  fs_border_width = border_width;
  fs_color = color;
  fs_border_color = border_color;
)";

const static std::string rect_fs = R"(
#version 330 core

in vec2 fs_position_ms;
in vec2 fs_radius;
in vec2 fs_border_width;
in vec4 fs_color;
in vec4 fs_border_color;

out vec4 color;

void main(){
  vec2 size = vec2(0.5, 0.5);
  vec2 pos = abs(fs_position_ms);
  vec2 arc_origin = size - fs_radius;
  vec2 arc_pos = pos - arc_origin;

  if (arc_pos.x >= 0 && arc_pos.y >= 0) {
    vec2 arc_pos_normalized = arc_pos / fs_radius;
    vec2 border_width_normalized = fs_border_width / fs_radius;
    r = length(arc_pos_normalized);
    float w = dot(arc_pos_normalized, border_width_normalized) / r;
    if (r > 1) {
      discard;
    } else if (r > 1 - w) {
      color = fs_border_color;
    } else {
      color = fs_color;
    }
  } else {
    if (pos.x > size.x || pos.y > size.y) {
      discard;
    }
    if (pos.x < size.x - fs_border_width.x
        || pos.y < size.y - fs_border_width.y) {
      color = fs_border_color;
    } else {
      color = fs_color;
    }
  }
}
)";

void Shape2dShader::init() {
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
        sizeof(Element),
        (void*)(offsetof(Element, M) + sizeof(float) * 3 * i));
    glVertexAttribDivisor(index, 1);
    glEnableVertexAttribArray(index);
    index++;
  }

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, radius));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      4,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, color));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, border_width));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      4,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, border_color));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static Mat3 make_transform(
    const Vec2& position,
    float angle,
    const Vec2& size) {
  Mat2 scale;
  scale(0, 0) = size.x;
  scale(1, 1) = size.y;
  Mat2 top_left = Rot2(angle).mat() * scale;

  Mat3 result;
  for (std::size_t i = 0; i < 2; i++) {
    for (std::size_t j = 0; j < 2; j++) {
      result(i, j) = top_left(i, j);
    }
  }
  result(0, 2) = position.x;
  result(1, 2) = position.y;
  result(2, 2) = 1;
  return result;
};

void Shape2dShader::queue_rect(
    const Vec2& position,
    float angle,
    const Vec2& size,
    const Color& color,
    float border_width,
    Color border_color) {
  Element element;
  element.M = make_transform(position, angle, size);
  element.radius = Vec2();
  element.color = color;
  element.border_width = Vec2::uniform(border_width) / size;
  element.border_color = border_color;
  elements.push_back(element);
}

void Shape2dShader::queue_circle(
    const Vec2& position,
    float radius,
    const Color& color,
    float border_width,
    Color border_color) {
  Element element;
  element.M = make_transform(position, 0, Vec2::uniform(2 * radius));
  element.radius = Vec2::uniform(0.5);
  element.color = color;
  element.border_width = Vec2::uniform(border_width / (2 * radius));
  element.border_color = border_color;
  elements.push_back(element);
}

void Shape2dShader::queue_ellipse(
    const Vec2& position,
    float angle,
    const Vec2& radii,
    const Color& color,
    float border_width,
    Color border_color) {
  Element element;
  element.M = make_transform(position, angle, 2 * radii);
  element.radius = Vec2::uniform(0.5);
  element.color = color;
  element.border_width = Vec2::uniform(border_width) / (2 * radii);
  element.border_color = border_color;
  elements.push_back(element);
}

void Shape2dShader::queue_line(
    const Vec2& a,
    const Vec2& b,
    float width,
    const Color& color,
    bool rounded_ends) {
  Vec2 position = (a + b) / 2;
  float angle = std::atan2(b.y - a.y, b.x - a.x);
  Vec2 size;
  Vec2 radius;
  if (rounded_ends) {
    size = Vec2((b - a).length() + width, width);
    radius = Vec2::uniform(width / 2);
  } else {
    size = Vec2((b - a).length(), width);
    radius = Vec2();
  }

  Element element;
  element.M = make_transform(position, angle, size);
  element.radius = radius / size;
  element.color = color;
  element.border_width = 0;
  elements.push_back(element);
}

void Shape2dShader::queue_capsule(
    const Vec2& start,
    const Vec2& end,
    float radius,
    const Color& color,
    float border_width,
    Color border_color) {
  Vec2 position = (start + end) / 2;
  float angle = std::atan2(end.y - start.y, end.x - start.x);
  Vec2 size = Vec2((start - end).length() + 2 * radius, 2 * radius);

  Element element;
  element.M = make_transform(position, angle, size);
  element.radius = Vec2::uniform(radius) / size;
  element.color = color;
  element.border_width = Vec2::uniform(border_width) / size;
  element.border_color = border_color;
  elements.push_back(element);
}

void Shape2dShader::draw(const Box2& viewport, const Camera2d& camera) {
  if (elements.empty()) {
    return;
  }
  glViewport(
      viewport.lower.x,
      viewport.lower.y,
      viewport.upper.x,
      viewport.upper.y);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  Mat3 V = camera.view_mat();
  Mat3 P = camera.projection_mat(viewport.size());
  Mat3 PV = P * V;

  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      elements.size() * sizeof(Element),
      elements.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUseProgram(program_id);
  glUniformMatrix3fv(uniform_PV, 1, GL_FALSE, PV.data);
  glBindVertexArray(VAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, static_vertex_count, elements.size());
}

void Shape2dShader::clear() {
  elements.clear();
}

} // namespace datagui
