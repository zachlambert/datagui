#include "datagui/visual/box_shader.hpp"
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
layout(location = 1) in vec2 offset;
layout(location = 2) in vec2 size;
layout(location = 4) in vec4 bg_color;
layout(location = 5) in vec4 border_color;
layout(location = 6) in float border_width;
layout(location = 7) in vec2 mask_lower;
layout(location = 8) in vec2 mask_upper;
layout(location = 9) in int z_index;

uniform vec2 viewport_size;
uniform int end_z_index;

out vec2 fs_offset;
out vec2 fs_half_width;
out vec2 fs_mask_offset;
out vec2 fs_mask_half_width;
out vec4 fs_bg_color;
out vec4 fs_border_color;
out float fs_border_width;

void main(){
  vec2 fs_pos = offset + vec2(vertex_pos.x * size.x, vertex_pos.y * size.y);
  gl_Position = vec4(
    -1 + 2*fs_pos.x/viewport_size.x,
    1 - 2*fs_pos.y/viewport_size.y,
    float(end_z_index - z_index) / end_z_index,
    1);

  fs_offset = fs_pos - (offset + size/2);
  fs_half_width = size / 2;
  fs_mask_offset = fs_pos - (mask_lower + mask_upper) / 2;
  fs_mask_half_width = (mask_upper - mask_lower) / 2;

  fs_bg_color = bg_color;
  fs_border_color = border_color;
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

static const std::array<Vec2, 6> quad_vertices = {
    Vec2(0.f, 1.f),
    Vec2(1.f, 0.f),
    Vec2(0.f, 0.f),
    Vec2(0.f, 1.f),
    Vec2(1.f, 0.f),
    Vec2(1.f, 1.f)};

void BoxShader::init() {
  gl_data.program_id = create_program(rect_vs, rect_fs);

  gl_data.uniform_viewport_size =
      glGetUniformLocation(gl_data.program_id, "viewport_size");
  gl_data.uniform_end_z_index =
      glGetUniformLocation(gl_data.program_id, "end_z_index");

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
      quad_vertices.size() * sizeof(Vec2),
      quad_vertices.data(),
      GL_STATIC_DRAW);

  GLuint index = 0;

  glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);
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

  glVertexAttribPointer(
      index,
      1,
      GL_INT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, z_index));
  glVertexAttribDivisor(index, 1);
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BoxShader::queue_box(
    const Box2& box,
    const Color& bg_color,
    float border_width,
    Color border_color,
    int z_index,
    const Box2& mask) {

  Element element;
  element.offset = box.lower;
  element.size = box.size();
  element.bg_color = bg_color;
  element.border_color = border_color;
  element.border_width = border_width;
  element.mask = mask;
  element.z_index = z_index;
  elements.push_back(element);
}

void BoxShader::render(const Vec2& viewport_size, int end_z_index) {
  glBindBuffer(GL_ARRAY_BUFFER, gl_data.instance_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      elements.size() * sizeof(Element),
      elements.data(),
      GL_STATIC_DRAW);

  glUseProgram(gl_data.program_id);
  glUniform2f(gl_data.uniform_viewport_size, viewport_size.x, viewport_size.y);
  glUniform1i(gl_data.uniform_end_z_index, end_z_index);
  glBindVertexArray(gl_data.VAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, quad_vertices.size(), elements.size());

  elements.clear();
}

} // namespace datagui
