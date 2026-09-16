#pragma once

#include <memory>
#include <vector>
#include "datagui/geometry/vec.hpp"

namespace dgui {

class ImageData {
public:
  struct Pixel {
    float r, g, b, a;

    void set(const Vec3& color, float alpha = 1.f) {
      r = color.x;
      g = color.y;
      b = color.z;
      a = 1.f;
    }
  };

  Pixel& operator()(size_t x, size_t y) {
    return pixels_[y * width_ + x];
  }
  const Pixel& operator()(size_t x, size_t y) const {
    return pixels_[y * width_ + x];
  }

  void resize(size_t width, size_t height);

  size_t width() const {
    return width_;
  }
  size_t height() const {
    return height_;
  }

private:
  size_t width_;
  size_t height_;
  std::vector<Pixel> pixels_;
};

class Image {
public:
  Image() = default;
  Image(const ImageData& data) {
    load(data);
  }

  void load(std::size_t width, std::size_t height, void* pixels);
  void load(const ImageData& data);

  bool is_loaded() const {
    return bool(data);
  }

  unsigned int texture() const {
    return data ? data->texture : 0;
  }

private:
  struct Data {
    std::size_t width;
    std::size_t height;
    unsigned int texture;

    Data() : width(0), height(0), texture(0) {}
    ~Data();
    Data(Data&&);
    Data& operator=(Data&&);

    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;
  };
  std::shared_ptr<Data> data;
};

} // namespace dgui
