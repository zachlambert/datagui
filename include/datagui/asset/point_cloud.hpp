#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry/vec.hpp"
#include <memory>

namespace dgui {

class PointCloud {
public:
  void load_colored_points(
      void* points,
      std::size_t num_points,
      std::size_t positions_offset,
      std::size_t colors_offset,
      std::size_t stride);

  void load_points(
      void* points,
      std::size_t num_points,
      std::size_t positions_offset,
      std::size_t stride,
      const Color& color);

  bool is_loaded() const {
    return bool(data);
  }

private:
  void init();

  struct Vertex {
    Vec3 position;
    Vec3 color;
  };

  struct Data {
    unsigned int VAO;
    unsigned int VBO;
    std::size_t vertex_count;

    Data() : VAO(0), VBO(0), vertex_count(0) {}
    ~Data();
    Data(Data&&);
    Data& operator=(const Data&);

    Data(const Data&) = delete;
    Data& operator=(Data&&) = delete;
  };
  std::shared_ptr<Data> data;

  friend class PointCloudShader;
};

} // namespace dgui
