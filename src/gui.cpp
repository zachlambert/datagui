#include "datagui/gui.hpp"

namespace datagui {

using namespace std::placeholders;

Gui::Gui(const Window::Config& config) :
    window(config),
    tree([this](const State& state) { systems[state.element_type].pop(state.element_index); }),
    renderers(font_manager),
    systems(font_manager) {
  renderers.text.init();
  renderers.geometry.init();
}

bool Gui::running() const {
  return window.running();
}

bool Gui::linear_layout() {
  tree.container_next([&](State& state) {
    // TODO
  });
  if (!tree.container_down()) {
    return false;
  }
  return true;
}

void Gui::container_end() {
  tree.up();
}

void Gui::text(const std::string& text, const std::function<void(TextStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::Text;
    state.element_index = systems.text.emplace();
    set_style(systems.text[state.element_index].style);
  });

  auto& state = tree[tree.current()].state;
  systems.text[state.element_index].text = text;
}

#if 0
DataPtr<std::string> Window::text_input(const std::string& initial_text) {
  tree.next();
  auto& node = tree[tree.current()];
  if (node.state_index == -1) {
    node.state_index = node_states.emplace();
    auto& node_state = node_states[node.state_index];
    node_state.element_type = ElementType::TextInput;
    node_state.element_index = text_input_states.emplace();
  }

  auto& state = text_input_states[node_states[node.state_index].element_index];
  state.text = initial_text;
}
#endif

void Gui::render_begin() {
  window.render_begin();
  tree.begin();
}

void Gui::render_end() {
  tree.end();

  window.render_end();
  renderers.geometry.render(window.size());
  renderers.text.render(window.size());

  window.poll_events();
}

} // namespace datagui
