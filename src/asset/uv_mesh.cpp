#include "datagui/asset/uv_mesh.hpp"
#include <GL/glew.h>
#include <vector>

namespace datagui {

UvMesh::Data::~Data() {
  if (VAO > 0) {
    glDeleteVertexArrays(1, &VAO);
  }
  if (VBO > 0) {
    glDeleteBuffers(1, &VBO);
  }
  if (EBO > 0) {
    glDeleteBuffers(1, &EBO);
  }
}

UvMesh::Data::Data(Data&& other) {
  VAO = other.VAO;
  VBO = other.VBO;
  EBO = other.VBO;
  index_count = other.index_count;
  other.VAO = 0;
  other.VBO = 0;
  other.EBO = 0;
  other.index_count = 0;
}

UvMesh::Data& UvMesh::Data::operator=(Data&& other) {
  if (VAO > 0) {
    glDeleteVertexArrays(1, &VAO);
  }
  if (VBO > 0) {
    glDeleteBuffers(1, &VBO);
  }
  if (EBO > 0) {
    glDeleteBuffers(1, &EBO);
  }
  VAO = other.VAO;
  VBO = other.VBO;
  EBO = other.VBO;
  index_count = other.index_count;
  other.VAO = 0;
  other.VBO = 0;
  other.EBO = 0;
  other.index_count = 0;
  return *this;
}

void UvMesh::init() {
  assert(!data);
  data = std::make_shared<Data>();

  assert(data->VAO == 0);
  assert(data->VBO == 0);
  assert(data->EBO == 0);

  glGenVertexArrays(1, &data->VAO);
  glGenBuffers(1, &data->VBO);
  glGenBuffers(1, &data->EBO);

  glBindVertexArray(data->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, data->VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->EBO);

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

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)(offsetof(Vertex, uv)));
  glEnableVertexAttribArray(index);
  index++;

  glBindVertexArray(0);
}

void UvMesh::load_vertices(
    const void* vertices,
    std::size_t num_vertices,
    std::size_t positions_offset,
    std::size_t normals_offset,
    std::size_t uvs_offset,
    std::size_t stride) {

  if (!data) {
    init();
  }

  std::vector<Vertex> gl_vertices(num_vertices);
  for (std::size_t i = 0; i < num_vertices; i++) {
    gl_vertices[i].position =
        *(Vec3*)((std::uint8_t*)vertices + i * stride + positions_offset);
    gl_vertices[i].normal =
        *(Vec3*)((std::uint8_t*)vertices + i * stride + normals_offset);
    gl_vertices[i].uv =
        *(Vec2*)((std::uint8_t*)vertices + i * stride + uvs_offset);
  }

  glBindBuffer(GL_ARRAY_BUFFER, data->VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      gl_vertices.size() * sizeof(Vertex),
      gl_vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UvMesh::load_indices(
    const unsigned int* const indices,
    std::size_t num_indices) {
  if (!data) {
    init();
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->EBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      num_indices * sizeof(unsigned int),
      indices,
      GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  data->index_count = num_indices;
}

void UvMesh::load_texture(
    std::size_t width,
    std::size_t height,
    void* texture_data) {
  if (!data) {
    init();
  }

  if (data->texture == 0) {
    glGenTextures(1, &data->texture);
  }
  glBindTexture(GL_TEXTURE_2D, data->texture);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      width,
      height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      texture_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace datagui
