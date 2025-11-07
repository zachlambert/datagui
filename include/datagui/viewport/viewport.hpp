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

protected:
  void bind_framebuffer() const;
  void unbind_framebuffer() const;

private:
  std::size_t width;
  std::size_t height;
  unsigned int texture;
  unsigned int framebuffer;

  friend class Gui;
};

} // namespace datagui
