#include "datagui/datapack/edit.hpp"
#include "datagui/element/key_list.hpp"
#include "datagui/gui.hpp"

namespace datagui {

bool datapack_edit(Gui& gui, const datapack::Schema& schema) {
  std::stack<datapack::Schema::Iterator> stack;
  struct ListState {
    Var<KeyList> keys;
    std::size_t i;
    bool children_open;
  };
  std::stack<ListState> list_stack;

  auto iter = schema.begin();
  bool changed = false;

  while (iter != schema.end()) {
    while (iter != schema.end() && !stack.empty()) {
      auto parent = stack.top();

      if (parent.object_begin()) {
        if (iter != parent.next()) {
          gui.section_end();
        }
        bool have_next = false;
        while (true) {
          if (iter.object_end()) {
            gui.series_end();
            stack.pop();
            iter = iter.next();
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
          have_next = true;
          iter = iter.next();
          break;
        }
        if (!have_next) {
          continue;
        }
        break;
      }
      if (parent.tuple_begin()) {
        if (iter.tuple_end()) {
          gui.series_end();
          stack.pop();
          iter = iter.next();
          continue;
        }
        if (!iter.tuple_next()) {
          throw datapack::SchemaError("Expected TupleNext");
        }
        iter = iter.next();
      }
      if (parent.list()) {
        assert(!list_stack.empty());
        auto& state = list_stack.top();

        if (state.children_open && state.i != 0) {
          if (gui.button("Remove")) {
            state.keys.mut().remove(state.i);
          } else {
            state.i++;
          }
          gui.series_end();
        }

        while (state.i < state.keys->size()) {
          gui.args_series().horizontal();
          if (gui.series_begin()) {
            break;
          }
          state.i++;
        }

        if (state.i == state.keys->size()) {
          if (state.children_open) {
            gui.series_end();
          }
          if (gui.button("new")) {
            state.keys.mut().append();
          }
          gui.series_end();
          stack.pop();
          list_stack.pop();
          continue;
        }
        iter = parent.next();
      }
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
    if (iter.list()) {
      if (gui.series_begin()) {
        stack.push(iter);
        auto keys = gui.variable<KeyList>();
        if (gui.series_begin()) {
          list_stack.push({keys, 0, true});
        } else {
          list_stack.push({keys, keys->size(), false});
        }
        iter = iter.next();
      }
      iter = iter.next().skip();
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
