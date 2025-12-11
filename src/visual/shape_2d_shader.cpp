#include "datagui/visual/shape_2d_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <array>
#include <assert.h>
#include <string>

namespace datagui {

const static std::string rect_vs = R"(
#version 330 core

layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec2 position;
layout(location = 2) in vec2 rotation_row1;
layout(location = 3) in vec2 rotation_row2;
layout(location = 4) in vec2 size;
layout(location = 5) in float radius;
layout(location = 6) in vec2 radius_scale;
layout(location = 7) in vec4 color;
layout(location = 8) in float border_width;
layout(location = 9) in vec4 border_color;
layout(location = 10) in vec2 mask_lower;
layout(location = 11) in vec2 mask_upper;

uniform vec2 viewport_size;

out vec2 fs_offset;
out vec2 fs_size;
out float fs_radius;
out vec2 fs_radius_scale;
out vec2 fs_mask_offset;
out vec2 fs_mask_size;
out vec4 fs_color;
out float fs_border_width;
out vec4 fs_border_color;

void main(){
  fs_offset = vec2(vertex_pos.x * size.x, vertex_pos.y * size.y);
  mat2 rotation = transpose(mat2(rotation_row1, rotation_row2));
  vec2 fs_pos = position + rotation * fs_offset;
  gl_Position = vec4(
    (fs_pos.x - viewport_size.x / 2) / (viewport_size.x / 2),
    (fs_pos.y - viewport_size.y / 2) / (viewport_size.y / 2),
    0,
    1);

  fs_size = size / 2;
  fs_radius = radius;
  fs_radius_scale = radius_scale;
  fs_border_width = border_width;
  fs_border_color = border_color;

  fs_mask_offset = fs_pos - (mask_lower + mask_upper) / 2;
  fs_mask_size = (mask_upper - mask_lower) / 2;

  fs_color = color;
}
)";

const static std::string rect_fs = R"(
#version 330 core

in vec2 fs_offset;
in vec2 fs_size;
in float fs_radius;
in vec2 fs_radius_scale;
in vec2 fs_mask_offset;
in vec2 fs_mask_size;
in vec4 fs_color;
in float fs_border_width;
in vec4 fs_border_color;

uniform vec2 viewport_size;

out vec4 color;

void main(){
  vec2 d_outer = (abs(fs_offset) - fs_size) / fs_radius_scale + fs_radius;
  vec2 d_inner = (abs(fs_offset) - max(fs_size-fs_border_width, 0)) /
    fs_radius_scale + max(fs_radius - fs_border_width, 0);

  float s_outer = length(max(d_outer, 0)) + min(max(d_outer.x, d_outer.y), 0);
  float s_inner = length(max(d_inner, 0)) + min(max(d_inner.x, d_inner.y), 0);

  float in_outer = float(s_outer <= fs_radius);
  float in_inner = float(s_inner <= max(fs_radius - fs_border_width, 0));

  vec2 mask_d = abs(fs_mask_offset) - fs_mask_size;
  float mask_s = length(max(mask_d, 0.0)) + min(max(mask_d.x, mask_d.y), 0.0);
  float in_mask = float(mask_s < 0);

  color = in_mask * (in_inner * fs_color + in_outer * (1-in_inner) * fs_border_color);
}
)";

static const std::array<Vec2, 6> quad_vertices = {
    Vec2(-0.5f, 0.5f),
    Vec2(0.5f, -0.5f),
    Vec2(-0.5f, -0.5f),
    Vec2(-0.5f, 0.5f),
    Vec2(0.5f, -0.5f),
    Vec2(0.5f, 0.5f)};

void Shape2dShader::init() {
  program_id = create_program(rect_vs, rect_fs);

  uniform_viewport_size = glGetUniformLocation(program_id, "viewport_size");

  // Generate ids
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &static_VBO);
  glGenBuffers(1, &instance_VBO);

  // Bind vertex array
  glBindVertexArray(VAO);

  // Bind and configure buffer for vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, static_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      quad_vertices.size() * sizeof(Vec2),
      quad_vertices.data(),
      GL_STATIC_DRAW);

  GLuint index = 0;

  glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Bind and configure buffer for indices
  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, position));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  // Row 1
  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, rotation));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  // Row 2
  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)(offsetof(Element, rotation) + 2 * sizeof(float)));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, size));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

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

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, radius_scale));
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
      1,
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

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)(offsetof(Element, mask) + offsetof(Box2, lower)));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)(offsetof(Element, mask) + offsetof(Box2, upper)));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Shape2dShader::queue_box(
    const Box2& box,
    const Color& color,
    float radius,
    float border_width,
    Color border_color,
    const Box2& mask) {

  Element element;
  element.position = box.center();
  element.rotation = Rot2();
  element.size = box.size();
  element.radius = radius;
  element.radius_scale = Vec2::ones();
  element.color = color;
  element.border_width = border_width;
  element.border_color = border_color;
  element.mask = mask;
  elements.push_back(element);
}

void Shape2dShader::queue_rect(
    const Vec2& position,
    float angle,
    const Vec2& size,
    const Color& color,
    float radius,
    float border_width,
    Color border_color,
    const Box2& mask) {
  Element element;
  element.position = position;
  element.rotation = Rot2(angle);
  element.size = size;
  element.radius = radius;
  element.radius_scale = Vec2::ones();
  element.color = color;
  element.border_width = border_width;
  element.border_color = border_color;
  element.mask = mask;
  elements.push_back(element);
}

void Shape2dShader::queue_capsule(
    const Vec2& start,
    const Vec2& end,
    float radius,
    const Color& color,
    float border_width,
    Color border_color,
    const Box2& mask) {
  Element element;
  element.position = (start + end) / 2;
  element.rotation = Rot2::line_rot(start, end);
  element.size = Vec2((start - end).length() + 2 * radius, 2 * radius);
  element.radius = radius;
  element.radius_scale = Vec2::ones();
  element.color = color;
  element.border_width = border_width;
  element.border_color = border_color;
  element.mask = mask;
  elements.push_back(element);
}

void Shape2dShader::queue_circle(
    const Vec2& position,
    float radius,
    const Color& color,
    float border_width,
    Color border_color,
    const Box2& mask) {
  Element element;
  element.position = position;
  element.rotation = Rot2();
  element.size = Vec2::uniform(2 * radius);
  element.radius = radius;
  element.radius_scale = Vec2::ones();
  element.color = color;
  element.border_width = border_width;
  element.border_color = border_color;
  element.mask = mask;
  elements.push_back(element);
}

void Shape2dShader::queue_ellipse(
    const Vec2& position,
    float angle,
    float x_radius,
    float y_radius,
    const Color& color,
    float border_width,
    Color border_color,
    const Box2& mask) {
  Element element;
  element.position = position;
  element.rotation = Rot2(angle);
  element.size = Vec2(2 * x_radius, 2 * y_radius);
  element.radius = std::min(x_radius, y_radius);
  element.radius_scale = Vec2(x_radius, y_radius) / element.radius;
  element.color = color;
  element.border_width = border_width;
  element.border_color = border_color;
  element.mask = mask;
  elements.push_back(element);
}

void Shape2dShader::queue_line(
    const Vec2& a,
    const Vec2& b,
    float width,
    const Color& color,
    const Box2& mask) {
  float angle = std::atan2(b.y - a.y, b.x - a.x);
  Element element;
  element.position = (a + b) / 2;
  element.rotation = Rot2(angle);
  element.size = Vec2((b - a).length() + width, width);
  element.radius = width / 2;
  element.radius_scale = Vec2::ones();
  element.color = color;
  element.border_width = 0;
  element.mask = mask;
  elements.push_back(element);
}

void Shape2dShader::draw(const Vec2& viewport_size) {
  if (elements.empty()) {
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      elements.size() * sizeof(Element),
      elements.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUseProgram(program_id);
  glUniform2f(uniform_viewport_size, viewport_size.x, viewport_size.y);
  glBindVertexArray(VAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, quad_vertices.size(), elements.size());

  elements.clear();
}

} // namespace datagui
