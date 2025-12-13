#pragma once

#include "datagui/geometry/box.hpp"
#include <vector>

namespace datagui {

class Image {
public:
  Image() : width(0), height(0), texture_(0) {}
  ~Image();
  Image(Image&&);

  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;
  Image& operator=(Image&&) = delete;

  void load(std::size_t width, std::size_t height, void* pixels);
  bool is_loaded() const {
    return texture_ != 0;
  }

private:
  unsigned int texture() const {
    return texture_;
  }

  std::size_t width;
  std::size_t height;
  unsigned int texture_;

  friend class ImageShader;
};

class ImageShader {
public:
  ImageShader();
  ~ImageShader();
  ImageShader(ImageShader&&);

  ImageShader(const ImageShader&) = delete;
  ImageShader& operator=(const ImageShader&) = delete;
  ImageShader& operator=(ImageShader&&) = delete;

  void init();

  // Y down
  void queue_image(
      const Image& image,
      const Vec2& position,
      float angle,
      const Vec2& size);

  void queue_image(
      const Image& image,
      const Vec2& position,
      const Vec2& size,
      const Box2& mask);

  // Y up
  void queue_texture(const Box2& box, int texture);

  void draw(const Vec2& viewport_size);
  void clear();

private:
  struct Vertex {
    Vec2 pos;
    Vec2 uv;
  };
  struct Command {
    std::vector<Vertex> vertices;
    int texture;
  };
  std::vector<Command> commands;

  unsigned int program_id;
  unsigned int uniform_viewport_size;
  unsigned int VAO;
  unsigned int VBO;
};

} // namespace datagui
