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
layout(location = 2) in vec4 transform_col1;
layout(location = 3) in vec4 transform_col2;
layout(location = 4) in vec4 transform_col3;
layout(location = 5) in vec4 transform_col4;
layout(location = 6) in vec4 color;

out vec3 fs_normal_cs;
out vec3 fs_normal_ws;
out vec4 fs_color;
flat out int fs_instance_id;

uniform mat4 P;
uniform mat4 V;

void main(){
  mat4 M = mat4(transform_col1, transform_col2, transform_col3, transform_col4);
  mat4 VM = V * M;
  mat4 PVM = P * VM;
  gl_Position = PVM * vec4(position, 1);
  fs_normal_cs = normalize((VM * vec4(normal, 0)).xyz);
  fs_normal_ws = normalize((M * vec4(normal, 0)).xyz);
  fs_color = color;
  fs_instance_id = gl_InstanceID;
}
)";

const static std::string shape_3d_fs = R"(
#version 330 core

in vec3 fs_normal_cs;
in vec3 fs_normal_ws;
in vec4 fs_color;
flat in int fs_instance_id;
out vec4 color;

void main(){
  const vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
  // https://godotshaders.com/shader/transparency-dither/
  // + Offset by fs_instance_id x some magic number
	if (fs_color.a < 0.001 ||
      fs_color.a < fract(magic.z * (0.05f * fs_instance_id + fract(dot(gl_FragCoord.xy, magic.xy))))) {
		discard;
	}

  float Ka = 0.2;
  float Kc = 0.5;
  float Kw = 0.3;
  float camera_light = clamp(dot(fs_normal_cs, vec3(0, 0, 1)), 0, 1);
  float world_light = clamp(dot(fs_normal_ws, vec3(0, 0, 1)), 0, 1);
  color = fs_color * min(Ka + Kc * camera_light + Kw * world_light, 1);
}
)";

struct Vertex {
  Vec3 position;
  Vec3 normal;
};

static void create_box(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices) {

  std::size_t start = vertices.size();
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
      indices.push_back(start + i * 4 + ind);
    }
    for (int ind = 0; ind < 3; ind++) {
      indices.push_back(start + (i + 1) * 4 - 1 - ind);
    }
  }
}

static void create_cylinder(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices) {

  float resolution = 0.1;
  const std::size_t N = std::max((size_t)std::ceil(2 * M_PI / resolution), 2lu);

  Vertex vertex;

  // Cylinder dimensions:
  // - Unit length, Unit radius
  // - Axis direction = +X
  // - Base is at origin, "Top" is at +X

  // End face

  std::size_t start = vertices.size();
  vertex.position = Vec3(1, 0, 0);
  vertex.normal = Vec3(1, 0, 0);
  vertices.push_back(vertex);
  for (std::size_t i = 0; i < N; i++) {
    float theta = i * 2 * M_PIf / N;
    vertex.position = Vec3(1, std::cos(theta), std::sin(theta));
    vertices.push_back(vertex);
  }
  for (std::size_t i = 0; i < N; i++) {
    indices.push_back(start);
    indices.push_back(start + 1 + i);
    indices.push_back(start + 1 + (i + 1) % N);
  }

  // Curved surface

  start = vertices.size();
  for (std::size_t i = 0; i < N; i++) {
    float theta = i * 2 * M_PIf / N;
    vertex.normal = Vec3(0, std::cos(theta), std::sin(theta));
    // End
    vertex.position = Vec3(1, std::cos(theta), std::sin(theta));
    vertices.push_back(vertex);
    // Base
    vertex.position = Vec3(0, std::cos(theta), std::sin(theta));
    vertices.push_back(vertex);
  }
  for (std::size_t i = 0; i < 2 * N; i += 2) {
    indices.push_back(start + i);
    indices.push_back(start + i + 1);
    indices.push_back(start + (i + 2) % (2 * N));
    indices.push_back(start + i + 1);
    indices.push_back(start + (i + 3) % (2 * N));
    indices.push_back(start + (i + 2) % (2 * N));
  }

  // Base face

  start = vertices.size();
  vertex.position = Vec3(0, 0, 0);
  vertex.normal = Vec3(-1, 0, 0);
  vertices.push_back(vertex);
  for (std::size_t i = 0; i < N; i++) {
    float theta = i * 2 * M_PIf / N;
    vertex.position = Vec3(0, std::cos(-theta), std::sin(-theta));
    vertices.push_back(vertex);
  }
  for (std::size_t i = 0; i < N; i++) {
    indices.push_back(start);
    indices.push_back(start + 1 + i);
    indices.push_back(start + 1 + (i + 1) % N);
  }
}

static void create_sphere(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices) {

  float resolution = 0.1;
  const size_t N = std::max((size_t)std::ceil(2 * M_PI / resolution), 2lu);
  std::size_t start = vertices.size();

  Vertex vertex;
  Vec3 direction;

  vertex.position = Vec3(0, 0, 1);
  vertex.normal = Vec3(0, 0, 1);
  vertices.push_back(vertex);
  for (std::size_t i = 1; i < N; i++) {
    float phi = (M_PIf * i) / N;
    direction.z = std::cos(phi);
    for (std::size_t j = 0; j < 2 * N; j++) {
      float theta = (2 * M_PIf * j) / (2 * N);
      direction.x = std::sin(phi) * std::cos(theta);
      direction.y = std::sin(phi) * std::sin(theta);
      vertex.position = direction;
      vertex.normal = direction;
      vertices.push_back(vertex);
    }
  }
  vertex.position = Vec3(0, 0, -1);
  vertex.normal = Vec3(0, 0, -1);
  vertices.push_back(vertex);

  // Top strip, common vertex = top
  for (std::size_t j = 0; j < 2 * N; j++) {
    indices.push_back(start);
    indices.push_back(start + 1 + j);
    if (j < 2 * N - 1) {
      indices.push_back(start + 2 + j);
    } else {
      indices.push_back(start);
    }
  }
  // Bottom strip, common vertex = bottom
  std::size_t bot = vertices.size() - 1;
  for (std::size_t j = 0; j < 2 * N; j++) {
    indices.push_back(bot);
    indices.push_back(bot - 1 - j);
    if (j < 2 * N - 1) {
      indices.push_back(bot - 2 - j);
    } else {
      indices.push_back(bot - 1);
    }
  }
  // Remaining strips, made up of rectangles between
  for (std::size_t i = 0; i < N - 2; i++) {
    std::size_t strip_1_start = start + 1 + 2 * N * i;
    std::size_t strip_2_start = start + 1 + 2 * N * (i + 1);
    for (std::size_t j = 0; j < 2 * N; j++) {
      // First triangle of rectangle
      indices.push_back(strip_1_start + j);
      indices.push_back(strip_2_start + j);
      if (j < 2 * N - 1) {
        indices.push_back(strip_2_start + j + 1);
      } else {
        indices.push_back(strip_2_start);
      }
      // Second triangle of rectangle
      if (j < 2 * N - 1) {
        indices.push_back(strip_2_start + j + 1);
        indices.push_back(strip_1_start + j + 1);
      } else {
        indices.push_back(strip_2_start);
        indices.push_back(strip_1_start);
      }
      indices.push_back(strip_1_start + j);
    }
  }
}

static void create_half_sphere(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices) {

  float resolution = 0.1;
  size_t N = std::max((size_t)std::ceil(2 * M_PI / resolution), 2lu);
  if (N % 2 != 0) {
    N++;
  }
  std::size_t start = vertices.size();

  Vertex vertex;
  Vec3 direction;

  vertex.position = Vec3(1, 0, 0);
  vertex.normal = Vec3(1, 0, 0);
  vertices.push_back(vertex);
  for (std::size_t i = 1; i <= N / 2; i++) {
    float phi = (M_PIf * i) / N;
    direction.x = std::cos(phi);
    for (std::size_t j = 0; j < 2 * N; j++) {
      float theta = (2 * M_PIf * j) / (2 * N);
      direction.y = std::sin(phi) * std::cos(theta);
      direction.z = std::sin(phi) * std::sin(theta);
      vertex.position = direction;
      vertex.normal = direction;
      vertices.push_back(vertex);
    }
  }

  // Top strip, common vertex = top
  for (std::size_t j = 0; j < 2 * N; j++) {
    indices.push_back(start);
    indices.push_back(start + 1 + j);
    if (j < 2 * N - 1) {
      indices.push_back(start + 2 + j);
    } else {
      indices.push_back(start);
    }
  }
  // Remaining strips, made up of rectangles between
  for (std::size_t i = 0; i < N / 2; i++) {
    std::size_t strip_1_start = start + 1 + 2 * N * i;
    std::size_t strip_2_start = start + 1 + 2 * N * (i + 1);
    for (std::size_t j = 0; j < 2 * N; j++) {
      // First triangle of rectangle
      indices.push_back(strip_1_start + j);
      indices.push_back(strip_2_start + j);
      if (j < 2 * N - 1) {
        indices.push_back(strip_2_start + j + 1);
      } else {
        indices.push_back(strip_2_start);
      }
      // Second triangle of rectangle
      if (j < 2 * N - 1) {
        indices.push_back(strip_2_start + j + 1);
        indices.push_back(strip_1_start + j + 1);
      } else {
        indices.push_back(strip_2_start);
        indices.push_back(strip_1_start);
      }
      indices.push_back(strip_1_start + j);
    }
  }
}

static void create_cone(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices) {

  float resolution = 0.1;
  const std::size_t N =
      std::max((size_t)std::ceil(2 * M_PIf / resolution), 2lu);

  Vertex vertex;

  // Base face

  std::size_t start = vertices.size();
  vertex.position = Vec3();
  vertex.normal = Vec3(-1, 0, 0);
  vertices.push_back(vertex);
  for (std::size_t i = 0; i < N; i++) {
    float theta = 2 * M_PIf * i / N;
    vertex.position = Vec3(0, std::cos(-theta), std::sin(-theta));
    vertices.push_back(vertex);
  }
  for (size_t i = 0; i < N; i++) {
    indices.push_back(start);
    indices.push_back(start + 1 + i);
    indices.push_back(start + 1 + (i + 1) % N);
  }

  // Curved surface

  start = vertices.size();
  for (std::size_t i = 0; i < N; i++) {
    float theta = 2 * M_PIf * i / N;
    float theta_plus_half = 2 * M_PI * (i + 0.5f) / N;
    vertex.position = Vec3(0, std::cos(theta), std::sin(theta));
    vertex.normal = Vec3(
        std::cos(M_PIf / 4),
        std::cos(theta) * std::sin(M_PIf / 4),
        std::sin(theta) * std::sin(M_PIf / 4));

    vertices.push_back(vertex);
    vertex.position = Vec3(1, 0, 0);
    vertex.normal = Vec3(
        std::cos(M_PIf / 4),
        std::cos(theta_plus_half) * std::sin(M_PIf / 4),
        std::sin(theta_plus_half) * std::sin(M_PIf / 4));
    vertices.push_back(vertex);
  }
  for (std::size_t i = 0; i < (2 * N); i += 2) {
    indices.push_back(start + i);
    indices.push_back(start + (i + 2) % (2 * N));
    indices.push_back(start + i + 1);
  }
}

static void create_plane(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices) {

  std::vector<Vec3> positions = {
      {-0.5, -0.5, 0},
      {0.5, -0.5, 0},
      {-0.5, 0.5, 0},
      {0.5, 0.5, 0},
  };

  std::size_t start = vertices.size();
  for (const auto& position : positions) {
    vertices.push_back(Vertex{position, Vec3::unit_z()});
  }
  for (const auto& position : positions) {
    vertices.push_back(Vertex{position, -Vec3::unit_z()});
  }

  indices.push_back(start);
  indices.push_back(start + 1);
  indices.push_back(start + 2);
  indices.push_back(start + 1);
  indices.push_back(start + 3);
  indices.push_back(start + 2);

  indices.push_back(start + 4);
  indices.push_back(start + 4 + 2);
  indices.push_back(start + 4 + 1);
  indices.push_back(start + 4 + 1);
  indices.push_back(start + 4 + 2);
  indices.push_back(start + 4 + 3);
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
    shape.indices_begin = indices.size();
    create_box(vertices, indices);
    shape.indices_end = indices.size();
  }
  {
    auto& shape = shapes[(std::size_t)ShapeType::Cylinder];
    shape.indices_begin = indices.size();
    create_cylinder(vertices, indices);
    shape.indices_end = indices.size();
  }
  {
    auto& shape = shapes[(std::size_t)ShapeType::Sphere];
    shape.indices_begin = indices.size();
    create_sphere(vertices, indices);
    shape.indices_end = indices.size();
  }
  {
    auto& shape = shapes[(std::size_t)ShapeType::HalfSphere];
    shape.indices_begin = indices.size();
    create_half_sphere(vertices, indices);
    shape.indices_end = indices.size();
  }
  {
    auto& shape = shapes[(std::size_t)ShapeType::Cone];
    shape.indices_begin = indices.size();
    create_cone(vertices, indices);
    shape.indices_end = indices.size();
  }
  {
    auto& shape = shapes[(std::size_t)ShapeType::Plane];
    shape.indices_begin = indices.size();
    create_plane(vertices, indices);
    shape.indices_end = indices.size();
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
  auto& element = elements[(std::size_t)ShapeType::Box].emplace_back();
  element.transform = make_transform(position, orientation, scale);
  element.color = color;
}

void Shape3dShader::queue_cylinder(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  auto& element = elements[(std::size_t)ShapeType::Cylinder].emplace_back();
  element.transform = make_transform(
      base_position,
      Rot3::line_rot(direction),
      Vec3(length, radius, radius));
  element.color = color;
}

void Shape3dShader::queue_sphere(
    const Vec3& position,
    float radius,
    const Color& color) {
  auto& element = elements[(std::size_t)ShapeType::Sphere].emplace_back();
  element.transform = make_transform(position, Rot3(), Vec3::uniform(radius));
  element.color = color;
}

void Shape3dShader::queue_half_sphere(
    const Vec3& position,
    const Vec3& direction,
    float radius,
    const Color& color) {
  auto& element = elements[(std::size_t)ShapeType::HalfSphere].emplace_back();
  element.transform = make_transform(
      position,
      Rot3::line_rot(direction),
      Vec3::uniform(radius));
  element.color = color;
}

void Shape3dShader::queue_cone(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  auto& element = elements[(std::size_t)ShapeType::Cone].emplace_back();
  element.transform = make_transform(
      base_position,
      Rot3::line_rot(direction),
      Vec3(length, radius, radius));
  element.color = color;
}

void Shape3dShader::queue_capsule(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color) {
  float length = std::max((end - start).length(), 0.f);
  if (length != 0.f) {
    Vec3 direction = (end - start) / length;
    queue_cylinder(start, direction, radius, length, color);
    queue_half_sphere(start, -direction, radius, color);
    queue_half_sphere(end, direction, radius, color);
  } else {
    queue_sphere(start, radius, color);
  }
}

void Shape3dShader::queue_arrow(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color,
    float head_length_scale,
    float head_radius_scale) {
  head_length_scale = std::max(head_length_scale, 1.f);
  head_radius_scale = std::max(head_radius_scale, 1.f);
  float length = std::max((end - start).length(), 0.f);
  float head_length = std::min(length, head_length_scale * 2 * radius);

  if (length == 0.f) {
    return;
  }

  Vec3 direction = (end - start) / length;
  float line_length = std::max(length - head_length, 0.f);
  if (line_length != 0.f) {
    queue_cylinder(start, direction, radius, line_length, color);
  }
  queue_cone(
      start + direction * line_length,
      direction,
      head_radius_scale * radius,
      head_length,
      color);
}

void Shape3dShader::queue_plane(
    const Vec3& position,
    const Rot3& orientation,
    const Vec2& scale,
    const Color& color) {
  auto& element = elements[(std::size_t)ShapeType::Plane].emplace_back();
  element.transform =
      make_transform(position, orientation, Vec3(scale.x, scale.y, 1));
  element.color = color;
}

void Shape3dShader::draw(const Box2& viewport, const Camera3d& camera) {
  glViewport(
      viewport.lower.x,
      viewport.lower.y,
      viewport.upper.x,
      viewport.upper.y);

  Mat4 V = camera.view_mat();
  Mat4 P = camera.projection_mat();

  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

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
        (shape.indices_end - shape.indices_begin),
        GL_UNSIGNED_INT,
        (void*)(sizeof(unsigned int) * shape.indices_begin),
        shape_elements.size());
  }
}

void Shape3dShader::clear() {
  for (auto& shape_elements : elements) {
    shape_elements.clear();
  }
}

} // namespace datagui
