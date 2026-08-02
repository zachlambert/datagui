#include "datagui/element/system.hpp"

#include "datagui/system/button.hpp"
#include "datagui/system/checkbox.hpp"
#include "datagui/system/collapsable.hpp"
#include "datagui/system/color_picker.hpp"
#include "datagui/system/dropdown.hpp"
#include "datagui/system/group.hpp"
#include "datagui/system/popup.hpp"
#include "datagui/system/select.hpp"
#include "datagui/system/slider.hpp"
#include "datagui/system/split.hpp"
#include "datagui/system/tabs.hpp"
#include "datagui/system/text_box.hpp"
#include "datagui/system/text_input.hpp"
#include "datagui/system/viewport_ptr.hpp"

namespace dgui {

void SystemSet::init(
    const std::shared_ptr<FontRegistry>& font_registry,
    const std::shared_ptr<Theme>& theme) {

  systems.resize((size_t)TypeCount);
#define REGISTER(Name, System, args...) \
  systems[(std::size_t)Type::Name] = std::make_unique<System>(args);

  REGISTER(Button, ButtonSystem, font_registry, theme);
  REGISTER(Checkbox, CheckboxSystem, font_registry, theme);
  REGISTER(Collapsable, CollapsableSystem, font_registry, theme);
  REGISTER(ColorPicker, ColorPickerSystem, font_registry, theme);
  REGISTER(Dropdown, DropdownSystem, font_registry, theme);
  REGISTER(Group, GroupSystem, theme);
  REGISTER(Popup, PopupSystem, font_registry, theme);
  REGISTER(Select, SelectSystem, font_registry, theme);
  REGISTER(Slider, SliderSystem, font_registry, theme);
  REGISTER(Split, SplitSystem, theme);
  REGISTER(Tabs, TabsSystem, font_registry, theme);
  REGISTER(TextBox, TextBoxSystem, font_registry, theme);
  REGISTER(TextInput, TextInputSystem, font_registry, theme);
  REGISTER(ViewportPtr, ViewportPtrSystem, theme);

#undef REGISTER
  for (const auto& system : systems) {
    assert(system);
  }
  popup_system = dynamic_cast<PopupSystem*>(systems[(size_t)Type::Popup].get());
}

void SystemSet::set_window_box(const Box2& box) {
  popup_system->set_window_box(box);
}

void SystemSet::set_input_state(ElementPtr element) {
  systems[(size_t)element.type()]->set_input_state(element);
}
void SystemSet::set_dependent_state(ElementPtr element) {
  systems[(size_t)element.type()]->set_dependent_state(element);
}

void SystemSet::render(ConstElementPtr element, DrawList& dl) {
  systems[(size_t)element.type()]->render(element, dl);
}

void SystemSet::render_content(ConstElementPtr element, DrawList& dl) {
  systems[(size_t)element.type()]->render_content(element, dl);
}

void SystemSet::mouse_event(ElementPtr element, const MouseEvent& event) {
  systems[(size_t)element.type()]->mouse_event(element, event);
}
void SystemSet::mouse_hover(ElementPtr element, const Vec2& mouse_pos) {
  systems[(size_t)element.type()]->mouse_hover(element, mouse_pos);
}
bool SystemSet::scroll_event(ElementPtr element, const ScrollEvent& event) {
  return systems[(size_t)element.type()]->scroll_event(element, event);
}
void SystemSet::key_event(ElementPtr element, const KeyEvent& event) {
  systems[(size_t)element.type()]->key_event(element, event);
}
void SystemSet::text_event(ElementPtr element, const TextEvent& event) {
  systems[(size_t)element.type()]->text_event(element, event);
}

void SystemSet::focus_enter(ElementPtr element) {
  systems[(size_t)element.type()]->focus_enter(element);
}
void SystemSet::focus_leave(ElementPtr element, bool success) {
  systems[(size_t)element.type()]->focus_leave(element, success);
}
void SystemSet::focus_tree_leave(ElementPtr element) {
  systems[(size_t)element.type()]->focus_tree_leave(element);
}

} // namespace dgui
