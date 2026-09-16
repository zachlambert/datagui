#pragma once

#include "datagui/input/event.hpp"
#include "datagui/theme.hpp"
#include "datagui/render/state/draw_list.hpp"
#include <memory>

namespace dgui {

class FontRegistry;

class Viewport {
public:
  virtual ~Viewport() = default;

  virtual void init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontRegistry>& font_registry) = 0;

  virtual void begin() = 0;
  virtual void end() {}
  virtual void draw(const Box2& viewport, DrawList& dl) = 0;

  virtual void mouse_event(const MouseEvent& event) {}
  virtual bool scroll_event(const ScrollEvent& event) {
    return false;
  }
};

} // namespace dgui
