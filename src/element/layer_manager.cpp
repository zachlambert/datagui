#include "datagui/element/layer_manager.hpp"
#include <queue>
#include <stack>

namespace dgui {

void LayerManager::visit(ElementPtr root) {
  struct FocusIndexCompare {
    bool operator()(const ElementPtr& lhs, const ElementPtr& rhs) const {
      return lhs.state().focus_index > rhs.state().focus_index;
    }
  };
  std::priority_queue<ElementPtr, std::deque<ElementPtr>, FocusIndexCompare>
      queue;
  queue.push(root);

  layers_.clear();
  while (!queue.empty()) {
    auto element = queue.top();
    queue.pop();
    if (element.state().display_mode == DisplayMode::Float) {
      layers_.emplace_back(element, false);
    }
    if (element.state().content_mode == DisplayMode::Float) {
      layers_.emplace_back(element, true);
    }

    std::stack<ElementPtr> stack;
    for (auto child = element.child(); child; child = child.next()) {
      stack.push(child);
    }
    while (!stack.empty()) {
      auto element = stack.top();
      stack.pop();
      if (element.state().display_mode == DisplayMode::Float ||
          element.state().content_mode == DisplayMode::Float) {
        queue.push(element);
        continue;
      }
      for (auto child = element.child(); child; child = child.next()) {
        stack.push(child);
      }
    }
  }
}

} // namespace dgui
