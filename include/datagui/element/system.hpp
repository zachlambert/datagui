#pragma once

#include "datagui/element/tree.hpp"
#include "datagui/geometry/box.hpp"
#include "datagui/geometry/vec.hpp"
#include "datagui/input/event.hpp"
#include "datagui/render/state/draw_list.hpp"
#include <vector>
#include <memory>

namespace dgui {

class System {
public:
  virtual ~System() = default;

  virtual void set_input_state(ElementPtr element) = 0;
  virtual void set_dependent_state(ElementPtr element) {};

  virtual void render(ConstElementPtr element, DrawList& dl) = 0;
  virtual void render_content(ConstElementPtr element, DrawList& dl) {}

  // Return true/false depending on if the event is handled
  virtual void mouse_event(ElementPtr element, const MouseEvent& event) {}
  virtual void mouse_hover(ElementPtr element, const Vec2& mouse_pos) {}
  virtual bool scroll_event(ElementPtr element, const ScrollEvent& event) {
    return false;
  }
  virtual void key_event(ElementPtr element, const KeyEvent& event) {}
  virtual void text_event(ElementPtr element, const TextEvent& event) {}

  // Node is focused via tab instead of clicking on it
  virtual void focus_enter(ElementPtr element) {}
  // Node is unfocused via tab, escape or clicking on another node
  // success = should the changes be retained?
  virtual void focus_leave(ElementPtr element, bool success) {}
  virtual void focus_tree_leave(ElementPtr element) {}
};

class FontRegistry;
class Theme;
class PopupSystem;

class SystemSet {
public:
  void init(const std::shared_ptr<FontRegistry>& font_registry, const std::shared_ptr<Theme>& theme);
  void set_window_box(const Box2& box);

  void set_input_state(ElementPtr element);
  void set_dependent_state(ElementPtr element);

  void render(ConstElementPtr element, DrawList& dl);
  void render_content(ConstElementPtr element, DrawList& dl);

  void mouse_event(ElementPtr element, const MouseEvent& event);
  void mouse_hover(ElementPtr element, const Vec2& mouse_pos);
  bool scroll_event(ElementPtr element, const ScrollEvent& event);
  void key_event(ElementPtr element, const KeyEvent& event);
  void text_event(ElementPtr element, const TextEvent& event);

  void focus_enter(ElementPtr element);
  void focus_leave(ElementPtr element, bool success);
  void focus_tree_leave(ElementPtr element);

private:
  std::vector<std::unique_ptr<System>> systems;
  PopupSystem* popup_system = nullptr;
};

} // namespace dgui
