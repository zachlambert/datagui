#pragma once

#include "datagui/geometry/vec.hpp"
#include <memory>

namespace dgui {

class UvMesh {
public:
  void load_vertices(
      const void* vertices,
      std::size_t num_vertices,
      std::size_t positions_offset,
      std::size_t normals_offset,
      std::size_t uv_offset,
      std::size_t stride);

  void load_indices(const unsigned int* const indices, std::size_t num_indices);

  void load_texture(std::size_t width, std::size_t height, void* texture_data);

  bool is_loaded() const {
    return bool(data) && data->VBO > 0 && data->EBO > 0 && data->texture > 0;
  }

private:
  struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
  };
  void init();

  struct Data {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    std::size_t index_count;
    unsigned int texture;

    Data() : VAO(0), VBO(0), EBO(0), index_count(0), texture(0) {}
    ~Data();
    Data(Data&&);
    Data& operator=(Data&&);
    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;
  };
  std::shared_ptr<Data> data;

  friend class UvMeshShader;
};

} // namespace dgui
