#pragma once

#include "datagui/geometry/box.hpp"
#include <vector>

namespace datagui {

class TextureShader {
public:
  TextureShader();
  ~TextureShader();
  TextureShader(TextureShader&&);

  TextureShader(const TextureShader&) = delete;
  TextureShader& operator=(const TextureShader&) = delete;
  TextureShader& operator=(TextureShader&&) = delete;

  void init();
  void queue_texture(const Box2& box, int texture);
  void draw(const Vec2& viewport_size);

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
