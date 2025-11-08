#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/shader.hpp"
#include <GL/glew.h>
#include <array>
#include <string>

namespace datagui {

const static std::string rect_vs = R"(
#version 330 core

// Input vertex data: position and normal
layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec2 offset;
layout(location = 2) in vec2 size;
layout(location = 3) in float radius;
layout(location = 4) in vec4 bg_color;
layout(location = 5) in vec4 border_color;
layout(location = 6) in float border_width;
layout(location = 7) in vec2 mask_lower;
layout(location = 8) in vec2 mask_upper;

uniform vec2 viewport_size;

out vec2 fs_offset;
out vec2 fs_half_width;
out vec2 fs_mask_offset;
out vec2 fs_mask_half_width;
out vec4 fs_bg_color;
out vec4 fs_border_color;
out float fs_radius;
out float fs_border_width;

void main(){
  vec2 fs_pos = offset + vec2(vertex_pos.x * size.x, vertex_pos.y * size.y);
  gl_Position = vec4(
    -1 + 2*fs_pos.x/viewport_size.x,
    1 - 2*fs_pos.y/viewport_size.y,
    0,
    1);

  fs_offset = fs_pos - (offset + size/2);
  fs_half_width = size / 2;
  fs_mask_offset = fs_pos - (mask_lower + mask_upper) / 2;
  fs_mask_half_width = (mask_upper - mask_lower) / 2;

  fs_bg_color = bg_color;
  fs_border_color = border_color;
  fs_radius = radius;
  fs_border_width = border_width;
}
)";

const static std::string rect_fs = R"(
#version 330 core

in vec2 fs_offset;
in vec2 fs_half_width;
in vec2 fs_mask_offset;
in vec2 fs_mask_half_width;
in vec4 fs_bg_color;
in vec4 fs_border_color;
in float fs_radius;
in float fs_border_width;

uniform vec2 viewport_size;

out vec4 color;

void main(){
  vec2 d = abs(fs_offset) - fs_half_width;
  float s = length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);

  vec2 mask_d = abs(fs_mask_offset) - fs_mask_half_width;
  float mask_s = length(max(mask_d, 0.0)) + min(max(mask_d.x, mask_d.y), 0.0);

  float in_mask = float(mask_s < 0);
  float in_border = float(s < 0 && s > -fs_border_width);
  float in_inside = float(s <= -fs_border_width);

  color = in_mask * (in_border * fs_border_color + in_inside * fs_bg_color);
}
)";

static const std::array<Vecf, 6> quad_vertices = {
    Vecf(0.f, 1.f),
    Vecf(1.f, 0.f),
    Vecf(0.f, 0.f),
    Vecf(0.f, 1.f),
    Vecf(1.f, 0.f),
    Vecf(1.f, 1.f)};

void GeometryRenderer::init() {
  gl_data.program_id = create_program(rect_vs, rect_fs);

  gl_data.uniform_viewport_size =
      glGetUniformLocation(gl_data.program_id, "viewport_size");

  // Generate ids
  glGenVertexArrays(1, &gl_data.VAO);
  glGenBuffers(1, &gl_data.static_VBO);
  glGenBuffers(1, &gl_data.instance_VBO);

  // Bind vertex array
  glBindVertexArray(gl_data.VAO);

  // Bind and configure buffer for vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, gl_data.static_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      quad_vertices.size() * sizeof(Vecf),
      quad_vertices.data(),
      GL_STATIC_DRAW);

  GLuint index = 0;

  glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(Vecf), (void*)0);
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Bind and configure buffer for indices
  glBindBuffer(GL_ARRAY_BUFFER, gl_data.instance_VBO);

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, offset));
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
      4,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, bg_color));
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
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)(offsetof(Element, mask) + offsetof(Boxf, lower)));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)(offsetof(Element, mask) + offsetof(Boxf, upper)));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GeometryRenderer::queue_box(
    const Boxf& box,
    const Color& bg_color,
    BoxDims border_width,
    Color border_color,
    float radius) {

  Element element;
  element.offset = box.lower;
  element.size = box.size();
  element.radius = radius;
  element.bg_color = bg_color;
  element.border_color = border_color;
  element.border_width = border_width.left; // TEMP: Replace argument with float
  if (masks.empty()) {
    element.mask = box;
  } else {
    element.mask = masks.top();
  }
  elements.push_back(element);
}

void GeometryRenderer::render(const Vecf& viewport_size) {
  glBindBuffer(GL_ARRAY_BUFFER, gl_data.instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      elements.size() * sizeof(Element),
      elements.data(),
      GL_STATIC_DRAW);

  glUseProgram(gl_data.program_id);
  glUniform2f(gl_data.uniform_viewport_size, viewport_size.x, viewport_size.y);
  glBindVertexArray(gl_data.VAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, quad_vertices.size(), elements.size());

  elements.clear();
}

} // namespace datagui
