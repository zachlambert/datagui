#pragma once

#include "datagui/input/event.hpp"
#include "datagui/theme.hpp"
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

  void init(
      std::size_t width,
      std::size_t height,
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm);

  Box2 viewport() const {
    return Box2(Vec2(), Vec2(width, height));
  }

  virtual void begin() = 0;
  virtual void end() = 0;
  virtual void mouse_event(const Vec2& size, const MouseEvent& event) {}
  virtual bool scroll_event(const Vec2& size, const ScrollEvent& event) {
    return false;
  }

  unsigned int texture() const {
    return texture_;
  }

protected:
  void bind_framebuffer(const Color& bg_color = Color::White());
  void unbind_framebuffer();

private:
  virtual void impl_init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) = 0;

  std::size_t width;
  std::size_t height;
  unsigned int texture_;
  unsigned int framebuffer;
  unsigned int render_buffer;
};

} // namespace datagui
