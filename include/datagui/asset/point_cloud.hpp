#pragma once

#include <memory>

namespace dgui {

class PointCloud {
public:
  void load(
      void* points,
      size_t point_count,
      size_t position_offset,
      size_t stride);

  void load_colored(
      void* points,
      size_t point_count,
      size_t position_offset,
      size_t color_offset,
      size_t stride);

  bool is_loaded() const {
    return bool(data);
  }

private:
  void load_impl(void* points, size_t point_count, size_t position_offset, size_t color_offset, size_t stride);

  struct Data {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    size_t vertex_count = 0;
    bool has_color = false;

    Data() = default;
    ~Data();
    Data(Data&&);
    Data& operator=(Data&&);
    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;
  };
  std::shared_ptr<Data> data;

  friend class PointCloudShader;
  friend class PointCloudProgram;
};

} // namespace dgui
