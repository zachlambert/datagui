#include "datagui/datapack/edit.hpp"
#include "datagui/gui.hpp"

namespace datagui {

bool datapack_edit(Gui& gui, const datapack::Schema& schema) {
  std::stack<datapack::Schema::Iterator> stack;
  auto iter = schema.begin();
  bool changed = false;

  while (iter != schema.end()) {
    while (!stack.empty()) {
      auto parent = stack.top();

      if (parent.object_begin()) {
        if (iter != parent.next()) {
          gui.section_end();
        }

        bool object_end = false;
        while (true) {
          if (iter.object_end()) {
            gui.series_end();
            stack.pop();
            iter = iter.next();
            object_end = true;
            break;
          }
          auto object_next = iter.object_next();
          if (!object_next) {
            throw datapack::SchemaError("Expected ObjectNext");
          }
          if (!gui.section_begin(object_next->key)) {
            iter = iter.next().skip();
            continue;
          }
          iter = iter.next();
          break;
        }
        if (object_end) {
          continue;
        } else {
          break;
        }
      }
      // TODO: Process others
      assert(false);
    }
    if (iter == schema.end()) {
      break;
    }

    if (iter.object_begin()) {
      if (gui.series_begin()) {
        stack.push(iter);
        iter = iter.next();
        continue;
      }
      iter = iter.skip();
      continue;
    }
    if (iter.number()) {
      if (gui.text_input("0")) {
        changed = true;
      }
      iter = iter.next();
      continue;
    }
    if (iter.string()) {
      if (gui.text_input("")) {
        changed = true;
      }
      iter = iter.next();
      continue;
    }
    if (iter.boolean()) {
      if (gui.checkbox(false)) {
        changed = true;
      }
      iter = iter.next();
      continue;
    }
    if (auto enumerate = iter.enumerate()) {
      if (gui.dropdown(enumerate->labels, -1)) {
        changed = true;
      }
      iter = iter.next();
      continue;
    }
    if (iter.binary()) {
      if (gui.text_input("")) {
        changed = true;
      }
      iter = iter.next();
      continue;
    }
  }
  return changed;
}

} // namespace datagui
