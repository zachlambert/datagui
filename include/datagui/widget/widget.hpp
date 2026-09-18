#pragma once

#include "datagui/input/event.hpp"
#include "datagui/render/state/draw_list.hpp"
#include "datagui/theme.hpp"
#include <memory>

namespace dgui {

class FontRegistry;

class Widget {
public:
  virtual ~Widget() = default;

  virtual void init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontRegistry>& font_registry) = 0;

  virtual void begin() = 0;
  virtual void end() {}
  virtual Vec2 min_size() const { return Vec2(); }
  virtual void set_dependent_state(const Box2& box) {}
  virtual void render(const Box2& box, DrawList& dl) const = 0;

  virtual void mouse_event(const Box2& box, const MouseEvent& event) {}
  virtual bool scroll_event(const Box2& box, const ScrollEvent& event) {
    return false;
  }
};

} // namespace dgui
