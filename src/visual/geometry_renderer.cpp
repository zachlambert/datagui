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
layout(location = 6) in vec4 border_width;

uniform vec2 viewport_size;

out vec2 fs_pos;
out vec2 fs_box_outer_lower;
out vec2 fs_box_outer_upper;
out vec2 fs_box_inner_lower;
out vec2 fs_box_inner_upper;
out float fs_radius;
out vec4 fs_bg_color;
out vec4 fs_border_color;

void main(){
  fs_pos = offset + vec2(vertex_pos.x * size.x, vertex_pos.y * size.y);
  gl_Position = vec4(
    -1 + 2*fs_pos.x/viewport_size.x,
    1 - 2*fs_pos.y/viewport_size.y,
    0,
    1);

  fs_box_outer_lower = offset;
  fs_box_outer_upper = offset + size;
  fs_box_inner_lower = offset + vec2(border_width[0], border_width[1]);
  fs_box_inner_upper = offset + size - vec2(border_width[2], border_width[3]);

  fs_radius = radius;
  fs_bg_color = bg_color;
  fs_border_color = border_color;
}
)";

const static std::string rect_fs = R"(
#version 330 core

in vec2 fs_pos;
in vec2 fs_box_outer_lower;
in vec2 fs_box_outer_upper;
in vec2 fs_box_inner_lower;
in vec2 fs_box_inner_upper;
in float fs_radius;
in vec4 fs_bg_color;
in vec4 fs_border_color;

uniform vec2 viewport_size;

out vec4 color;

void main(){
    vec2 delta_outer_lower = fs_pos - fs_box_outer_lower;
    vec2 delta_outer_upper = fs_box_outer_upper - fs_pos;
    vec2 delta_outer = vec2(
      min(delta_outer_lower.x, delta_outer_upper.x),
      min(delta_outer_lower.y, delta_outer_upper.y));

    float dist_outer = 0;
    if (delta_outer.x <= fs_radius && delta_outer.y <= fs_radius) {
        dist_outer = fs_radius - length(vec2(fs_radius, fs_radius) - delta_outer);
    } else {
        dist_outer = min(delta_outer.x, delta_outer.y);
    }

    vec2 delta_inner_lower = fs_pos - fs_box_inner_lower;
    vec2 delta_inner_upper = fs_box_inner_upper - fs_pos;
    vec2 delta_inner = vec2(
      min(delta_inner_lower.x, delta_inner_upper.x),
      min(delta_inner_lower.y, delta_inner_upper.y));

    float dist_inner = 0;
    if (delta_inner.x <= fs_radius && delta_inner.y <= fs_radius) {
        dist_inner = fs_radius - length(vec2(fs_radius, fs_radius) - delta_inner);
    } else {
        dist_inner = min(delta_inner.x, delta_inner.y);
    }

    if (dist_outer < 0) {
        color = vec4(0, 0, 0, 0);
    } else if (dist_inner < 0) {
        color = fs_border_color;
    } else {
        color = fs_bg_color;
    }
}
)";

static const std::array<Vecf, 6> quad_vertices = {
    Vecf(0.f, 1.f),
    Vecf(1.f, 0.f),
    Vecf(0.f, 0.f),
    Vecf(0.f, 1.f),
    Vecf(1.f, 0.f),
    Vecf(1.f, 1.f)};

GeometryRenderer::GeometryRenderer() {}

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
      4,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Element),
      (void*)offsetof(Element, border_width));
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
  element.offset = box.lower; //  + border_width.offset();
  element.size = box.size();  //  - border_width.size();
  element.radius = radius;
  element.bg_color = bg_color;
  element.border_color = border_color;
  element.border_width = border_width;
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
