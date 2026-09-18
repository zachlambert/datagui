#include "datagui/asset/mesh.hpp"
#include <GL/glew.h>
#include <cassert>

namespace dgui {

Mesh::Data::~Data() {
  // Ignores zero values
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &EBO);
  glDeleteBuffers(1, &VBO);
}

Mesh::Data::Data(Data&& other) {
  VAO = other.VAO;
  other.VAO = 0;
  EBO = other.EBO;
  other.EBO = 0;
  VBO = other.VBO;
  other.VBO = 0;
  index_count = other.index_count;
  has_color = other.has_color;
  has_uv = other.has_uv;
  texture = std::move(other.texture);
}

Mesh::Data& Mesh::Data::operator=(Data&& other) {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &EBO);
  glDeleteBuffers(1, &VBO);

  VAO = other.VAO;
  other.VAO = 0;
  EBO = other.EBO;
  other.EBO = 0;
  VBO = other.VBO;
  other.VBO = 0;
  has_color = other.has_color;
  has_uv = other.has_uv;
  texture = std::move(other.texture);
  index_count = other.index_count;

  return *this;
}

void Mesh::load_impl(
    const unsigned int* indices,
    size_t index_count,
    const void* vertices,
    size_t vertex_count,
    size_t pos_offset,
    size_t normal_offset,
    size_t color_offset,
    size_t uv_offset,
    int stride) {
  if (data) {
    data.reset();
  }
  data = std::make_shared<Data>();

  data->index_count = index_count;

  glGenVertexArrays(1, &data->VAO);
  glGenBuffers(1, &data->VBO);
  glGenBuffers(1, &data->EBO);

  // Assign EBO to VAO
  glBindVertexArray(data->VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->EBO);
  glBindVertexArray(0);

  // Assign index data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->EBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      index_count * sizeof(unsigned int),
      indices,
      GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
        (void*)pos_offset);
    glEnableVertexAttribArray(index);
    index++;

    glVertexAttribPointer(
        index,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*)normal_offset);
    glEnableVertexAttribArray(index);
    index++;

    if (color_offset > 0) {
      glVertexAttribPointer(
          index,
          4,
          GL_FLOAT,
          GL_FALSE,
          stride,
          (void*)color_offset);
      glEnableVertexAttribArray(index);
    }
    index++;

    if (uv_offset > 0) {
      glVertexAttribPointer(
          index,
          2,
          GL_FLOAT,
          GL_FALSE,
          stride,
          (void*)uv_offset);
      glEnableVertexAttribArray(index);
    }
    index++;
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Copy vertex cata
  glBindBuffer(GL_ARRAY_BUFFER, data->VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      vertex_count * stride,
      vertices,
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::load(
    const unsigned int* indices,
    size_t index_count,
    const void* vertices,
    size_t vertex_count,
    size_t pos_offset,
    size_t normal_offset,
    size_t stride) {
  load_impl(
      indices,
      index_count,
      vertices,
      vertex_count,
      pos_offset,
      normal_offset,
      0,
      0,
      stride);
}

void Mesh::load_colored(
    const unsigned int* indices,
    size_t index_count,
    const void* vertices,
    size_t vertex_count,
    size_t pos_offset,
    size_t normal_offset,
    size_t color_offset,
    size_t stride) {
  load_impl(
      indices,
      index_count,
      vertices,
      vertex_count,
      pos_offset,
      normal_offset,
      color_offset,
      0,
      stride);
  data->has_color = true;
}

void Mesh::load_textured(
    const unsigned int* indices,
    size_t index_count,
    const void* vertices,
    size_t vertex_count,
    size_t pos_offset,
    size_t normal_offset,
    size_t uv_offset,
    size_t stride,
    Image texture) {
  load_impl(
      indices,
      index_count,
      vertices,
      vertex_count,
      pos_offset,
      normal_offset,
      0,
      uv_offset,
      stride);
  data->texture = texture;
  data->has_uv = true;
}

} // namespace dgui
