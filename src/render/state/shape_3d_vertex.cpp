#include "datagui/render/state/shape_3d_vertex.hpp"

namespace dgui {

namespace {

void make_box(
    std::vector<Shape3dVertex>& vertices,
    std::vector<unsigned int>& indices) {

  std::size_t start = vertices.size();
  Shape3dVertex vertex;

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

void make_cylinder(
    std::vector<Shape3dVertex>& vertices,
    std::vector<unsigned int>& indices) {

  float resolution = 0.1;
  const std::size_t N = std::max((size_t)std::ceil(2 * M_PI / resolution), 2lu);

  Shape3dVertex vertex;

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

void make_sphere(
    std::vector<Shape3dVertex>& vertices,
    std::vector<unsigned int>& indices) {

  float resolution = 0.5;
  const size_t N = std::max((size_t)std::ceil(2 * M_PI / resolution), 2lu);
  std::size_t start = vertices.size();

  Shape3dVertex vertex;
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
      indices.push_back(start + 1);
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

void make_half_sphere(
    std::vector<Shape3dVertex>& vertices,
    std::vector<unsigned int>& indices) {

  float resolution = 0.1;
  size_t N = std::max((size_t)std::ceil(2 * M_PI / resolution), 2lu);
  if (N % 2 != 0) {
    N++;
  }
  std::size_t start = vertices.size();

  Shape3dVertex vertex;
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

void make_cone(
    std::vector<Shape3dVertex>& vertices,
    std::vector<unsigned int>& indices) {

  float resolution = 0.1;
  const std::size_t N =
      std::max((size_t)std::ceil(2 * M_PIf / resolution), 2lu);

  Shape3dVertex vertex;

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

void make_plane(
    std::vector<Shape3dVertex>& vertices,
    std::vector<unsigned int>& indices) {

  std::vector<Vec3> positions = {
      {-0.5, -0.5, 0},
      {0.5, -0.5, 0},
      {-0.5, 0.5, 0},
      {0.5, 0.5, 0},
  };

  std::size_t start = vertices.size();
  for (const auto& position : positions) {
    vertices.push_back(Shape3dVertex{position, Vec3::unit_z()});
  }
  for (const auto& position : positions) {
    vertices.push_back(Shape3dVertex{position, -Vec3::unit_z()});
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

} // namespace

void add_shape_vertices(
    Shape3dType type,
    std::vector<Shape3dVertex>& vertices,
    std::vector<unsigned int>& indices) {
  switch (type) {
    case Shape3dType::Box:
      make_box(vertices, indices);
      break;
    case Shape3dType::Cylinder:
      make_cylinder(vertices, indices);
      break;
    case Shape3dType::Sphere:
      make_sphere(vertices, indices);
      break;
    case Shape3dType::HalfSphere:
      make_half_sphere(vertices, indices);
      break;
    case Shape3dType::Cone:
      make_cone(vertices, indices);
      break;
    case Shape3dType::Plane:
      make_plane(vertices, indices);
      break;
  }
}

} // namespace dgui
