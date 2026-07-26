#pragma once

#include "datagui/asset/image.hpp"
#include <memory>

namespace dgui {

class Mesh {
public:
  void load(
      const unsigned int* indices,
      size_t index_count,
      const void* vertices,
      size_t vertex_count,
      size_t pos_offset,
      size_t normal_offset,
      size_t stride);

  void load_colored(
      const unsigned int* indices,
      size_t index_count,
      const void* vertices,
      size_t vertex_count,
      size_t pos_offset,
      size_t normal_offset,
      size_t color_offset,
      size_t stride);

  void load_textured(
      const unsigned int* indices,
      size_t index_count,
      const void* vertices,
      size_t vertex_count,
      size_t pos_offset,
      size_t normal_offset,
      size_t uv_offset,
      size_t stride,
      Image texture);

  bool is_loaded() const {
    return bool(data);
  }

private:
  void load_impl(
      const unsigned int* indices,
      size_t index_count,
      const void* vertices,
      size_t vertex_count,
      size_t pos_offset,
      size_t normal_offset,
      size_t color_offset,
      size_t uv_offset,
      int stride);

  struct Data {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    std::size_t index_count = 0;
    bool has_color = false;
    bool has_uv = false;
    Image texture;

    Data() = default;
    ~Data();
    Data(Data&&);
    Data& operator=(Data&&);
    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;
  };
  std::shared_ptr<Data> data;

  friend class MeshShader;
  friend class MeshProgram;
};

} // namespace dgui
