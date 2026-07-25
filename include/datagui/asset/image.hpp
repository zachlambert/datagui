#pragma once

#include <memory>

namespace dgui {

class Image {
public:
  void load(std::size_t width, std::size_t height, void* pixels);

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

  // TODO: Remove friend
  friend class ImageShader;
};

} // namespace dgui
