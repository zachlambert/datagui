#include "datagui/system/widget_ptr.hpp"

namespace dgui {

void WidgetPtrSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& widget_ptr = element.widget_ptr();

  state.fixed_size = widget_ptr.widget->min_size();

  if (auto width = std::get_if<LengthFixed>(&widget_ptr.width)) {
    state.fixed_size.x = std::min(state.fixed_size.x, width->value);
    state.dynamic_size.x = 0;
  } else if (auto width = std::get_if<LengthDynamic>(&widget_ptr.width)) {
    state.dynamic_size.x = std::max(state.dynamic_size.x, width->weight);
  }
  if (auto height = std::get_if<LengthFixed>(&widget_ptr.height)) {
    state.fixed_size.y = height->value;
    state.dynamic_size.y = 0;
  } else if (auto height = std::get_if<LengthDynamic>(&widget_ptr.height)) {
    state.dynamic_size.y = std::max(state.dynamic_size.y, height->weight);
  }
}

void WidgetPtrSystem::set_dependent_state(ElementPtr element) {
  const auto& state = element.state();
  const auto& widget_ptr = element.widget_ptr();
  widget_ptr.widget->set_dependent_state(state.box());
}

void WidgetPtrSystem::render(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  const auto& widget_ptr = element.widget_ptr();
  widget_ptr.widget->render(state.box(), dl);
}

void WidgetPtrSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  const auto& widget_ptr = element.widget_ptr();
  widget_ptr.widget->mouse_event(state.box(), event);
}

bool WidgetPtrSystem::scroll_event(
    ElementPtr element,
    const ScrollEvent& event) {
  const auto& state = element.state();
  const auto& widget_ptr = element.widget_ptr();
  return widget_ptr.widget->scroll_event(state.box(), event);
}

} // namespace dgui
