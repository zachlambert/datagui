#include "datagui/asset/point_cloud.hpp"
#include <GL/glew.h>
#include <vector>

namespace dgui {

PointCloud::Data::~Data() {
  if (VAO > 0) {
    glDeleteVertexArrays(1, &VAO);
  }
  if (VBO > 0) {
    glDeleteBuffers(1, &VBO);
  }
}

PointCloud::Data::Data(Data&& other) {
  VAO = other.VAO;
  VBO = other.VBO;
  vertex_count = other.vertex_count;
  other.VAO = 0;
  other.VBO = 0;
  other.vertex_count = 0;
}

void PointCloud::init() {
  assert(!data);
  data = std::make_shared<Data>();
  assert(data->VAO == 0);
  assert(data->VBO == 0);

  glGenVertexArrays(1, &data->VAO);
  glGenBuffers(1, &data->VBO);

  glBindVertexArray(data->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, data->VBO);

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
      (void*)(offsetof(Vertex, color)));
  glEnableVertexAttribArray(index);
  index++;

  glBindVertexArray(0);
}

void PointCloud::load_colored_points(
    void* points,
    std::size_t num_points,
    std::size_t positions_offset,
    std::size_t colors_offset,
    std::size_t stride) {

  if (!data) {
    init();
  }

  std::vector<Vertex> gl_points(num_points);
  for (std::size_t i = 0; i < num_points; i++) {
    gl_points[i].position =
        *(Vec3*)((std::uint8_t*)points + i * stride + positions_offset);
    gl_points[i].color =
        *(Vec3*)((std::uint8_t*)points + i * stride + colors_offset);
  }

  glBindBuffer(GL_ARRAY_BUFFER, data->VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      gl_points.size() * sizeof(Vertex),
      gl_points.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  data->vertex_count = num_points;
}

void PointCloud::load_points(
    void* points,
    std::size_t num_points,
    std::size_t positions_offset,
    std::size_t stride,
    const Color& color) {

  if (!data) {
    init();
  }

  std::vector<Vertex> gl_points(num_points);
  for (std::size_t i = 0; i < num_points; i++) {
    gl_points[i].position =
        *(Vec3*)((std::uint8_t*)points + i * stride + positions_offset);
    gl_points[i].color.x = color.r * 255;
    gl_points[i].color.y = color.g * 255;
    gl_points[i].color.z = color.b * 255;
  }

  glBindBuffer(GL_ARRAY_BUFFER, data->VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      gl_points.size() * sizeof(Vertex),
      gl_points.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  data->vertex_count = num_points;
}

} // namespace dgui
