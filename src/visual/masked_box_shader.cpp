#include "datagui/visual/masked_box_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <assert.h>
#include <string>

namespace datagui {

const static std::string vertex_shader = R"(
#version 330 core

layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec3 box_lower;
layout(location = 2) in vec3 box_upper;
layout(location = 3) in vec3 mask_lower;
layout(location = 4) in vec2 mask_upper;
layout(location = 5) in vec4 color;
layout(location = 6) in vec4 border_color;
layout(location = 7) in float border_width;
layout(location = 8) in float radius;

uniform vec2 viewport_lower;
uniform vec2 viewport_upper;

out vec2 fs_position;
flat out vec2 fs_box_lower;
flat out vec2 fs_box_upper;
flat out vec2 fs_mask_lower;
flat out vec2 fs_mask_upper;
flat out vec4 fs_color;
flat out vec4 fs_border_color;
flat out float fs_border_width;
flat out float fs_radius;

void main(){
  mat3 M = mat3(M_col1, M_col2, M_col3);

  fs_position = box_lower + vertex_pos * (box_upper - box_lower),
  vec2 viewport_center = (viewport_lower + viewport_upper) / 2;
  vec2 viewport_half_size = (viewport_upper - viewport_lower) / 2;
  gl_Position = vec4(
    (position - viewport_center) / viewport_half_size
    0,
    1
  );

  fs_box_lower = box_lower;
  fs_box_upper = box_upper;
  fs_mask_lower = mask_lower;
  fs_mask_upper = mask_upper;
  fs_color = color;
  fs_border_color = border_color;
  fs_border_width = border_width;
  fs_radius = radius;
)";

const static std::string fragment_shader = R"(
#version 330 core

in vec2 fs_position;
flat in vec2 fs_box_lower;
flat in vec2 fs_box_upper;
flat in vec2 fs_mask_lower;
flat in vec2 fs_mask_upper;
flat in vec4 fs_color;
flat in vec4 fs_border_color;
flat in float fs_border_width;
flat in float fs_radius;

out vec4 color;

void main(){
  if (fs_position.x < fs_mask_lower.x
      || fs_position.x > fs_mask_upper.x
      || fs_position.y < fs_mask_lower.y
      || fs_position.y > fs_mask_upper.y) {
    discard;
  }

  float x = min(
    fs_position.x - fs_box_lower.x,
    fs_box_upper.x - fs_position.x);
  float y = min(
    fs_position.y - fs_box_lower.y,
    fs_box_upper.y - fs_position.y);

  // Signed distance, +ve = outside, -ve = inside
  float s;
  if (x <= radius && y <= radius) {
    s = radius - hypot(radius - x, radius - y);
  } else {
    s = -max(x, y);
  }
  if (s > 0) {
    color = vec4(0, 0, 0, 0);
  } else if (s > -fs_border_width) {
    color = fs_border_color;
  } else {
    color = fs_color;
  }
}
)";

void MaskedBoxShader::init() {
  program_id = create_program(vertex_shader, fragment_shader);

  uniform_viewport_lower = glGetUniformLocation(program_id, "viewport_lower");
  uniform_viewport_upper = glGetUniformLocation(program_id, "viewport_upper");

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
      {Vec2(0.f, 1.f)},
      {Vec2(1.f, 0.f)},
      {Vec2(0.f, 0.f)},
      {Vec2(0.f, 1.f)},
      {Vec2(1.f, 0.f)},
      {Vec2(1.f, 1.f)}};
  static_vertex_count = static_vertices.size();

  // Bind and configure buffer for vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, static_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      static_vertices.size() * sizeof(Vertex),
      static_vertices.data(),
      GL_STATIC_DRAW);

  GLuint index = 0;

  // vertex_pos
  glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);

  // box_lower
  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)(offsetof(Element, box) + offsetof(Box2, lower)));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  // box_upper
  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)(offsetof(Element, box) + offsetof(Box2, lower)));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  // mask_lower
  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)(offsetof(Element, box) + offsetof(Box2, lower)));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  // mask_upper
  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)(offsetof(Element, box) + offsetof(Box2, lower)));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  // color
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

  // border_color
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

  // border_width
  glVertexAttribPointer(
      index,
      1,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, border_width));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  // radius
  glVertexAttribPointer(
      index,
      1,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, radius));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MaskedBoxShader::queue_box(
    const Box2& mask,
    const Box2& box,
    const Color& color,
    float border_width,
    Color border_color,
    float radius) {
  auto& element = elements.emplace_back();
  element.mask = mask;
  element.box = box;
  element.color = color;
  element.border_width = border_width;
  element.border_color = border_color;
  element.radius = radius;
}

void MaskedBoxShader::draw(const Box2& viewport) {
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

  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      elements.size() * sizeof(Element),
      elements.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUseProgram(program_id);

  glUniform2fv(uniform_viewport_lower, 1, &viewport.lower.x);
  glUniform2fv(uniform_viewport_upper, 1, &viewport.upper.x);

  glBindVertexArray(VAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, static_vertex_count, elements.size());
}

void MaskedBoxShader::clear() {
  elements.clear();
}

} // namespace datagui
