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

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 transform_row1;
layout(location = 3) in vec4 transform_row2;
layout(location = 4) in vec4 transform_row3;
layout(location = 5) in vec4 transform_row4;
layout(location = 6) in vec4 color;

out vec3 fs_position_cs;
out vec3 fs_normal_cs;
out vec4 fs_color;

uniform mat4 P;
uniform mat4 V;

void main(){
  mat4 M = transpose(mat4(transform_row1, transform_row2, transform_row3, transform_row4));
  mat4 VM = transpose(V) * M;
  mat4 PVM = transpose(P) * VM;
  gl_Position = PVM * vec4(position, 1);
  fs_normal_cs = normalize((VM * vec4(normal, 0)).xyz);
  fs_color = vec4(1, 0, 0, 1); // color;
}
)";

const static std::string shape_3d_fs = R"(
#version 330 core

in vec3 fs_normal_cs;
in vec4 fs_color;
out vec4 color;

uniform float ambient;

void main(){
  // Light as if the light comes from the camera view
  float cos_theta = clamp(dot(fs_normal_cs, vec3(0, 0, 1)), 0, 1);
  color = fs_color * (ambient + (1 - ambient) * cos_theta);
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
      indices.push_back(vertices_offset + i * 4 + ind);
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
  uniform_ambient = glGetUniformLocation(program_id, "ambient");

  // Generate ids
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &static_VBO);
  glGenBuffers(1, &static_EBO);
  glGenBuffers(1, &instance_VBO);

  // Bind vertex array
  glBindVertexArray(VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_EBO);

  // Bind and configure buffer for vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, static_VBO);

  GLuint index = 0;

  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)(offsetof(Vertex, position)));
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
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

void Shape3dShader::draw(const Vec2& viewport_size, const Camera3d& camera) {
  Mat4 V = camera.view_mat();
  Mat4 P = camera.projection_mat(viewport_size.x / viewport_size.y);

  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glUseProgram(program_id);
  glUniformMatrix4fv(uniform_V, 1, GL_FALSE, V.data);
  glUniformMatrix4fv(uniform_P, 1, GL_FALSE, P.data);
  glUniform1f(uniform_ambient, args.ambient);

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
        (void*)(sizeof(unsigned int) * shape.indices_offset),
        shape_elements.size());

    shape_elements.clear();
  }
}

} // namespace datagui
