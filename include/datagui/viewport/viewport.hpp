#pragma once

#include "datagui/geometry/box.hpp"
#include <cstddef>
#include <memory>

namespace datagui {

class FontManager;

class Viewport {
public:
  Viewport();
  ~Viewport();
  Viewport(Viewport&&);

  Viewport(const Viewport&) = delete;
  Viewport& operator=(const Viewport&) = delete;
  Viewport& operator=(Viewport&&) = delete;

protected:
  Vec2 framebuffer_size() const {
    return Vec2(width, height);
  }

private:
  void init(
      std::size_t width,
      std::size_t height,
      const std::shared_ptr<FontManager>& fm);
  void render(const Box2& bounds);

  virtual void begin() = 0;
  virtual void impl_init(const std::shared_ptr<FontManager>& fm) = 0;
  virtual void impl_render() = 0;

  std::size_t width;
  std::size_t height;
  unsigned int texture;
  unsigned int framebuffer;

  struct Vertex {
    Vec2 pos;
    Vec2 uv;
  };
  unsigned int program_id;
  unsigned int uniform_viewport_size;
  unsigned int VAO;
  unsigned int VBO;

  friend class Gui;
  friend class ViewportSystem;
};

} // namespace datagui
