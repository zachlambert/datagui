#pragma once

#include "datagui/asset/image.hpp"
#include "datagui/geometry/box.hpp"
#include "datagui/geometry/camera.hpp"
#include <memory>
#include <vector>

namespace dgui {

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

  void queue_masked_image(
      const Box2& mask,
      const Image& image,
      const Vec2& position,
      const Vec2& size);

  // Y up

  void queue_viewport(const Box2& mask, const Box2& box, int texture);

  void draw(const Box2& viewport, const Camera2d& camera);
  void clear();

private:
  struct Vertex {
    Vec2 pos;
    Vec2 uv;
  };
  struct Command {
    Image image;
    int texture = 0; // Image or texture used
    std::vector<Vertex> vertices;
  };
  std::vector<Command> commands;

  unsigned int program_id;
  unsigned int uniform_PV;
  unsigned int VAO;
  unsigned int VBO;
};

} // namespace dgui
