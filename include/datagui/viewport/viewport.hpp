#pragma once

#include "datagui/input/event.hpp"
#include "datagui/theme.hpp"
#include <cstddef>
#include <memory>

namespace dgui {

class FontManager;

class Viewport {
public:
  virtual void init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) = 0;

  virtual void begin() = 0;
  virtual void end() {}
  virtual void draw(const Box2& viewport, const Box2& mask) = 0;

  virtual void mouse_event(const MouseEvent& event) {}
  virtual bool scroll_event(const ScrollEvent& event) {
    return false;
  }
};

} // namespace dgui
