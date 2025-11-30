#pragma once

#include "datagui/geometry/box.hpp"
#include <cstddef>

namespace datagui {

class OpenglRgbImage {
public:
  OpenglRgbImage() : width(0), height(0), texture_(0) {}
  ~OpenglRgbImage();
  OpenglRgbImage(OpenglRgbImage&&);

  OpenglRgbImage(const OpenglRgbImage&) = delete;
  OpenglRgbImage& operator=(const OpenglRgbImage&) = delete;
  OpenglRgbImage& operator=(OpenglRgbImage&&) = delete;

  void write(std::size_t width, std::size_t height, void* pixels);
  unsigned int texture() const {
    return texture_;
  }

private:
  std::size_t width;
  std::size_t height;
  unsigned int texture_;
};

class ImageShader {
public:
  ~ImageShader();
  ImageShader(ImageShader&&);

  ImageShader(const ImageShader&) = delete;
  ImageShader& operator=(const ImageShader&) = delete;
  ImageShader& operator=(ImageShader&&) = delete;

  void init();
  void draw(int texture, const Box2& box);
  void draw(int texture, const Vec2& position, double angle, const Vec2& size);

private:
  struct Vertex {
    Vec2 pos;
    Vec2 uv;
  };
  unsigned int program_id;
  unsigned int uniform_viewport_size;
  unsigned int VAO;
  unsigned int VBO;
};

} // namespace datagui
