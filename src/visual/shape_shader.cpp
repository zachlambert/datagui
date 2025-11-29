#include "datagui/visual/shape_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <array>
#include <assert.h>
#include <string>

namespace datagui {

const static std::string rect_vs = R"(
#version 330 core

// Input vertex data: position and normal
layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec2 position;
layout(location = 2) in vec2 rotation_row1;
layout(location = 3) in vec2 rotation_row2;
layout(location = 4) in vec2 size;
layout(location = 5) in vec2 radius;
layout(location = 6) in vec4 color;
layout(location = 7) in vec2 mask_lower;
layout(location = 8) in vec2 mask_upper;

uniform float y_dir;
uniform vec2 viewport_size;

out vec2 fs_offset;
out vec2 fs_size;
out vec2 fs_radius;
out vec2 fs_mask_offset;
out vec2 fs_mask_size;
out vec4 fs_color;

void main(){
  fs_offset = vec2(vertex_pos.x * size.x, vertex_pos.y * size.y);
  mat2 rotation = transpose(mat2(rotation_row1, rotation_row2));
  vec2 fs_pos = position + rotation * fs_offset;
  gl_Position = vec4(
    (fs_pos.x - viewport_size.x / 2) / (viewport_size.x / 2),
    y_dir * (fs_pos.y - viewport_size.y / 2) / (viewport_size.y / 2),
    0,
    1);

  fs_size = size / 2;
  fs_radius = radius;

  fs_mask_offset = fs_pos - (mask_lower + mask_upper) / 2;
  fs_mask_size = (mask_upper - mask_lower) / 2;

  fs_color = color;
}
)";

const static std::string rect_fs = R"(
#version 330 core

in vec2 fs_offset;
in vec2 fs_size;
in vec2 fs_radius;
in vec2 fs_mask_offset;
in vec2 fs_mask_size;
in vec4 fs_color;

uniform vec2 viewport_size;

out vec4 color;

void main(){
  vec2 u = abs(fs_offset) - (fs_size - fs_radius);
  // (x/a)**2 + (y/b)**2 <= 1
  // x**2 * b**2 + y**2 * a**2 <= a**2 * b**2
  // Allow it to be negative =>
  // max(x, 0)**2 + ...

  float lhs_x = u.x*u.x * fs_radius.y*fs_radius.y;
  float lhs_y = u.y*u.y * fs_radius.x*fs_radius.x;
  float rhs = fs_radius.x*fs_radius.x * fs_radius.y*fs_radius.y;

  float in_shape = float((lhs_x + lhs_y) <= rhs);

  vec2 mask_d = abs(fs_mask_offset) - fs_mask_size;
  float mask_s = length(max(mask_d, 0.0)) + min(max(mask_d.x, mask_d.y), 0.0);
  float in_mask = float(mask_s < 0);

  color = in_mask * in_shape * fs_color;
}
)";

static const std::array<Vec2, 6> quad_vertices = {
    Vec2(-0.5f, 0.5f),
    Vec2(0.5f, -0.5f),
    Vec2(-0.5f, -0.5f),
    Vec2(-0.5f, 0.5f),
    Vec2(0.5f, -0.5f),
    Vec2(0.5f, 0.5f)};

void ShapeShader::init() {
  program_id = create_program(rect_vs, rect_fs);

  uniform_y_dir = glGetUniformLocation(program_id, "y_dir");
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

void ShapeShader::Command::queue_box(
    const Box2& box,
    const Color& color,
    float radius,
    float border_width,
    Color border_color,
    const Box2& mask) {

  if (border_width > 0) {
    queue_box(box, color, radius, 0, Color::Black(), mask);
  }
  Element element;
  element.position = box.center();
  element.rotation = Rot2();
  element.size = box.size() - Vec2::uniform(border_width * 2);
  element.radius = Vec2::uniform(radius);
  element.color = color;
  element.mask = mask;
  elements.push_back(element);
}

void ShapeShader::Command::queue_rect(
    const Vec2& position,
    float angle,
    const Vec2& size,
    const Color& color,
    float radius,
    float border_width,
    Color border_color,
    const Box2& mask) {
  if (border_width > 0) {
    queue_rect(
        position,
        angle,
        size,
        border_color,
        radius,
        0,
        Color::Black(),
        mask);
  }
  Element element;
  element.position = position;
  element.rotation = Rot2(angle);
  element.size = size - Vec2::uniform(border_width);
  element.radius = Vec2::uniform(radius);
  element.color = color;
  element.mask = mask;
  elements.push_back(element);
}

void ShapeShader::Command::queue_capsule(
    const Vec2& start,
    const Vec2& end,
    float radius,
    const Color& color,
    float border_width,
    Color border_color,
    const Box2& mask) {
  if (border_width > 0) {
    queue_capsule(start, end, radius, border_color, 0, Color::Black(), mask);
  }
  Element element;
  element.position = (start + end) / 2;
  element.rotation = Rot2::line_rot(start, end);
  element.size = Vec2(
      (start - end).length() + 2 * radius - 2 * border_width,
      2 * radius - 2 * border_width);
  element.radius = Vec2::uniform(radius);
  element.color = color;
  element.mask = mask;
  elements.push_back(element);
}

void ShapeShader::Command::queue_circle(
    const Vec2& position,
    float radius,
    const Color& color,
    float border_width,
    Color border_color,
    const Box2& mask) {
  if (border_width > 0) {
    queue_circle(position, radius, border_color, 0, Color::Black(), mask);
  }
  Element element;
  element.position = position;
  element.rotation = Rot2();
  element.size = Vec2::uniform(radius);
  element.radius = Vec2::uniform(radius);
  element.color = color;
  element.mask = mask;
  elements.push_back(element);
}

void ShapeShader::Command::queue_ellipse(
    const Vec2& position,
    float angle,
    float x_radius,
    float y_radius,
    const Color& color,
    float border_width,
    Color border_color,
    const Box2& mask) {
  if (border_width > 0) {
    queue_ellipse(
        position,
        angle,
        x_radius,
        y_radius,
        border_color,
        0,
        Color::Black(),
        mask);
  }
  Element element;
  element.position = position;
  element.rotation = Rot2(angle);
  element.size =
      Vec2(2 * x_radius, 2 * y_radius) - Vec2::uniform(2 * border_width);
  element.radius = Vec2(x_radius, y_radius);
  element.color = color;
  element.mask = mask;
  elements.push_back(element);
}

void ShapeShader::draw(
    const Command& command,
    float y_dir,
    const Vec2& viewport_size) {
  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      command.elements.size() * sizeof(Element),
      command.elements.data(),
      GL_STATIC_DRAW);

  glUseProgram(program_id);
  glUniform1f(uniform_y_dir, y_dir);
  glUniform2f(uniform_viewport_size, viewport_size.x, viewport_size.y);
  glBindVertexArray(VAO);
  glDrawArraysInstanced(
      GL_TRIANGLES,
      0,
      quad_vertices.size(),
      command.elements.size());
}

} // namespace datagui
