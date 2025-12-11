#include "datagui/visual/shape_3d_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>

namespace datagui {

static Mat4 make_transform(
    const Vec3& position,
    const Rot3& orientation,
    const Vec3& scale) {
  Mat3 scale_mat;
  for (std::size_t i = 0; i < 3; i++) {
    scale_mat(i, i) = scale(i);
  }
  Mat3 top_left = orientation.mat() * scale_mat;

  Mat4 transform;
  transform(3, 3) = 1;
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; j++) {
      transform(i, j) = top_left(i, j);
    }
  }
  for (std::size_t i = 0; i < 3; i++) {
    transform(i, 3) = position(i);
  }
  return transform;
}

const static std::string shape_3d_vs = R"(
#version 330 core

// Input vertex data: position and normal
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

const static std::string shape_3d_fs = R"(
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

struct Vertex {
  Vec3 position;
  Vec3 normal;
};

static std::size_t create_box(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices) {

  std::size_t vertices_offset = vertices.size();
  std::size_t indices_offset = indices.size();
  Vertex vertex;

  for (int i = 0; i < 6; i++) {
    int dim = i % 3;
    int dim2 = (dim + 1) % 3;
    int dim3 = (dim + 2) % 3;

    float dir = i / 3 == 0 ? -1 : 1;

    vertex.normal(dim) = dir;
    vertex.normal(dim2) = 0;
    vertex.normal(dim3) = 0;

    vertex.position(dim) = dir / 2;
    for (int j = 0; j < 4; j++) {
      int j_ = i / 3 == 0 ? (3 - j) : j;
      float dir2 = j_ % 2 == 0 ? -1 : 1;
      float dir3 = (j_ / 2 == 0 ? -1 : 1) * dir;
      vertex.position(dim2) = dir2 / 2;
      vertex.position(dim3) = dir3 / 2;

      vertices.push_back(vertex);
    }

    for (int ind = 0; ind < 3; ind++) {
      indices.push_back(vertices_offset + (i + 1) * 4 - 1 - ind);
    }
    for (int ind = 0; ind < 3; ind++) {
      indices.push_back(vertices_offset + (i + 1) * 4 - 1 - ind);
    }
  }
  return indices.size() - indices_offset;
}

void Shape3dShader::init() {
  // =============================================================
  // Initialise shader

  program_id = create_program(shape_3d_vs, shape_3d_fs);

  uniform_P = glGetUniformLocation(program_id, "P");
  uniform_V = glGetUniformLocation(program_id, "V");

  // Generate ids
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &static_VBO);
  glGenBuffers(1, &static_EBO);
  glGenBuffers(1, &instance_VBO);

  // Bind vertex array
  glBindVertexArray(VAO);

  // Bind and configure buffer for vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, static_VBO);

  GLuint index = 0;

  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vec3),
      (void*)(offsetof(Vertex, position)));
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vec3),
      (void*)(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(index);
  index++;

  // Bind and configure buffer for instance attributes
  glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);

  for (std::size_t i = 0; i < 4; i++) {
    // Row i
    glVertexAttribPointer(
        index,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Element),
        (void*)(offsetof(Element, transform) + sizeof(float) * 4 * i));
    glVertexAttribDivisor(index, 1);
    glEnableVertexAttribArray(index);
    index++;
  }

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

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // =============================================================
  // Allocate buffer data

  shapes.resize(ShapeTypeCount);
  elements.resize(ShapeTypeCount);

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  {
    auto& shape = shapes[(std::size_t)ShapeType::Box];
    shape.indices_offset = indices.size();
    shape.index_count = create_box(vertices, indices);
  }

  glBindBuffer(GL_ARRAY_BUFFER, static_VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      vertices.size() * sizeof(Vertex),
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

void Shape3dShader::queue_box(
    const Vec3& position,
    const Rot3& orientation,
    const Vec3& scale,
    const Color& color) {
  elements[(std::size_t)ShapeType::Box].push_back(
      {make_transform(position, orientation, scale), color});
}

void Shape3dShader::draw(const Camera3d& camera) {
  Mat4 V = camera.view_mat();
  Mat4 P = camera.projection_mat();

  glUseProgram(program_id);
  glUniformMatrix4fv(uniform_V, 1, GL_FALSE, V.data);
  glUniformMatrix4fv(uniform_P, 1, GL_FALSE, P.data);

  glBindVertexArray(VAO);

  for (std::size_t shape_i = 0; shape_i < ShapeTypeCount; shape_i++) {
    const auto& shape = shapes[shape_i];
    auto& shape_elements = elements[shape_i];
    if (shape_elements.empty()) {
      continue;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        shape_elements.size() * sizeof(Element),
        shape_elements.data(),
        GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawElementsInstanced(
        GL_TRIANGLES,
        shape.index_count,
        GL_UNSIGNED_INT,
        (void*)shape.indices_offset,
        shape_elements.size());

    shape_elements.clear();
  }
}

} // namespace datagui
