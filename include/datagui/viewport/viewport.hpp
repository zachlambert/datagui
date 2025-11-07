#pragma once

#include <cstddef>

namespace datagui {

class Viewport {
public:
  Viewport();
  ~Viewport();

  Viewport(const Viewport&) = delete;
  Viewport(Viewport&&) = default;
  Viewport& operator=(const Viewport&) = delete;
  Viewport& operator=(Viewport&&) = delete;

  void init(std::size_t width, std::size_t height);
  void update();

private:
  virtual void render() = 0;

  std::size_t width;
  std::size_t height;
  unsigned int texture;
  unsigned int framebuffer;

  friend class Gui;
};

} // namespace datagui
