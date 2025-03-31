#include "datagui/gui.hpp"
#include <stack>

namespace datagui {

using namespace std::placeholders;

Gui::Gui(const Window::Config& config) :
    window(config),
    text_renderer(font_manager),
    linear_layout_system(geometry_renderer),
    text_system(font_manager, text_renderer),
    text_input_system(font_manager, text_renderer, geometry_renderer),
    tree([this](const State& state) {
      element_system(state.element_type).pop(state.element_index);
    }) {
  geometry_renderer.init();
  text_renderer.init();
}

bool Gui::running() const {
  return window.running();
}

bool Gui::linear_layout(
    const std::function<void(LinearLayoutStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::LinearLayout;
    state.element_index = linear_layout_system.emplace();
    if (set_style) {
      set_style(linear_layout_system[state.element_index].style);
    }
  });
  if (!tree.container_down()) {
    return false;
  }
  return true;
}

void Gui::container_end() {
  tree.up();
}

void Gui::text(
    const std::string& text,
    const std::function<void(TextStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::Text;
    state.element_index = text_system.emplace();
    if (set_style) {
      set_style(text_system[state.element_index].style);
    }
  });

  auto node = tree.current();
  text_system[node->element_index].text = text;
}

Tree::ConstData<std::string> Gui::text_input(
    const std::function<void(TextInputStyle&)>& set_style) {
  tree.container_next([&](State& state) {
    state.element_type = ElementType::TextInput;
    state.element_index = text_input_system.emplace();
    if (set_style) {
      set_style(text_input_system[state.element_index].style);
    }
  });

  auto node = tree.current();
  return tree.data_current<std::string>();
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

  // Calculate size components
  {
    struct State {
      Tree::Ptr node;
      bool first_visit;
      State(Tree::Ptr node) : node(node), first_visit(true) {}
    };
    std::stack<State> stack;
    stack.emplace(tree.root());

    while (!stack.empty()) {
      State& state = stack.top();
      auto node = state.node;

      if (!node.visible() || node->element_index == -1) {
        stack.pop();
        continue;
      }

      // If the node has children, process these first
      if (node.first_child() && state.first_visit) {
        state.first_visit = false;
        auto child = node.first_child();
        while (child) {
          stack.emplace(child);
          child = child.next();
        }
        continue;
      }
      stack.pop();

      element_system(node->element_type).set_layout_input(node);
    }
  }

  {
    std::stack<Tree::Ptr> stack;
    stack.emplace(tree.root());
    tree.root()->size = window.size();

    while (!stack.empty()) {
      auto node = stack.top();
      stack.pop();

      if (!node.visible() || node->element_index == -1) {
        continue;
      }

      if (!node.first_child()) {
        continue;
      }

      element_system(node->element_type).set_child_layout_output(node);

      auto child = node.first_child();
      while (child) {
        stack.push(child);
        child = child.next();
      }
    }
  }

  {
    std::stack<Tree::ConstPtr> stack;
    stack.push(tree.root());

    while (!stack.empty()) {
      auto node = stack.top();
      stack.pop();

      if (!node.visible() || node->element_index == -1) {
        continue;
      }
      element_system(node->element_type).render(node);

      auto child = node.first_child();
      while (child) {
        stack.push(child);
        child = child.next();
      }
    }
  }

  geometry_renderer.render(window.size());
  text_renderer.render(window.size());
  window.render_end();

  window.poll_events();
}

ElementSystem& Gui::element_system(ElementType type) {
  switch (type) {
  case ElementType::LinearLayout:
    return linear_layout_system;
  case ElementType::Text:
    return text_system;
  case ElementType::TextInput:
    return text_system;
  case ElementType::Undefined:
    assert(false);
    return linear_layout_system;
  }
  assert(false);
  return linear_layout_system;
}

} // namespace datagui
