#include <datagui/window.hpp>

void list_1(datagui::Window& window) {
  if (!window.vertical_layout()) {
    return;
  }

  auto items = window.data<std::vector<std::string>>({});

  if (window.horizontal_layout()) {
    auto item = window.data<std::string>();
    if (auto value = window.text_input("")) {
      item = *value;
    }
    if (window.button("Add")) {
      items.mut().push_back(*item);
    }
    window.container_end();
  }

  std::size_t i = 0;
  while (i < items->size()) {
    std::size_t next_i = i + 1;
    if (window.horizontal_layout()) {
      window.text((*items)[i]);
      if (window.button("Remove")) {
        items.mut().erase(items->begin() + i);
        next_i = i;
      }
      window.container_end();
    }
    i = next_i;
  }

  window.container_end();
}

void list_2(datagui::Window& window) {
  if (!window.vertical_layout()) {
    return;
  }

  auto items = window.data<std::vector<std::string>>({});

  if (window.button("Add")) {
    items.mut().push_back("");
  }

  std::size_t i = 0;
  while (i < items->size()) {
    std::size_t next_i = i + 1;
    if (window.horizontal_layout()) {
      if (auto value = window.text_input()) {
        items.mut()[i] = *value;
      }
      if (window.button("Remove")) {
        items.mut().erase(items->begin() + i);
        next_i = i;
      }
      window.container_end();
    }
    i = next_i;
  }

  window.container_end();
}

int main() {
  datagui::Window::Config config;
  config.title = "Datagui Example 1";
  datagui::Style style;

  datagui::Window window(config, style);

  while (window.running()) {
    window.render_begin();
    if (window.vertical_layout()) {
      list_1(window);
      list_2(window);
      window.container_end();
    }
    window.render_end();
  }
  return 0;
}
