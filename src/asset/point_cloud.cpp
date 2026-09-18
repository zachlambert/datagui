#include "datagui/asset/point_cloud.hpp"
#include <GL/glew.h>
#include <cassert>

namespace dgui {

PointCloud::Data::~Data() {
  // Ignores zero values
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

PointCloud::Data::Data(Data&& other) {
  VAO = other.VAO;
  other.VAO = 0;
  VBO = other.VBO;
  other.VBO = 0;
  vertex_count = other.vertex_count;
  has_color = other.has_color;
}

PointCloud::Data& PointCloud::Data::operator=(Data&& other) {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  VAO = other.VAO;
  other.VAO = 0;
  VBO = other.VBO;
  other.VBO = 0;
  vertex_count = other.vertex_count;
  has_color = other.has_color;

  return *this;
}

void PointCloud::load_impl(
    void* points,
    size_t point_count,
    size_t position_offset,
    size_t color_offset,
    size_t stride) {
  if (data) {
    data.reset();
  }
  data = std::make_shared<Data>();
  data->vertex_count = point_count;
  data->has_color = color_offset > 0;

  glGenVertexArrays(1, &data->VAO);
  glGenBuffers(1, &data->VBO);

  // Configure vertex array

  glBindVertexArray(data->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, data->VBO);
  {
    GLuint index = 0;

    glVertexAttribPointer(
        index,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*)position_offset);
    glEnableVertexAttribArray(index);
    index++;

    if (color_offset > 0) {
      glVertexAttribPointer(
          index,
          3,
          GL_FLOAT,
          GL_FALSE,
          stride,
          (void*)color_offset);
      glEnableVertexAttribArray(index);
    }
    index++;
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Copy point data
  glBindBuffer(GL_ARRAY_BUFFER, data->VBO);
  glBufferData(GL_ARRAY_BUFFER, point_count * stride, points, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PointCloud::load(
    void* points,
    size_t point_count,
    size_t position_offset,
    size_t stride) {
  load_impl(points, point_count, position_offset, 0, stride);
}

void PointCloud::load_colored(
    void* points,
    size_t point_count,
    size_t position_offset,
    size_t color_offset,
    size_t stride) {
  load_impl(points, point_count, position_offset, color_offset, stride);
}

} // namespace dgui
