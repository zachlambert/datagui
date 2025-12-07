#include "datagui/datapack/edit.hpp"
#include "datagui/element/key_list.hpp"
#include "datagui/gui.hpp"

namespace datagui {

void datapack_edit(
    Gui& gui,
    const std::string& root_label,
    const datapack::Schema& schema) {
  std::stack<datapack::Schema::Iterator> stack;
  struct ListState {
    Var<KeyList> keys;
    std::size_t i;
  };
  std::stack<ListState> list_stack;

  auto iter = schema.begin();
  std::string next_label = root_label;

  while (iter != schema.end()) {
    while (iter != schema.end() && !stack.empty()) {
      auto parent = stack.top();

      if (parent.object_begin()) {
        bool have_next = false;
        while (true) {
          if (iter.object_end()) {
            gui.end();
            stack.pop();
            iter = iter.next();
            break;
          }
          auto object_next = iter.object_next();
          if (!object_next) {
            throw datapack::SchemaError("Expected ObjectNext");
          }
          next_label = object_next->key;
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
          gui.end();
          stack.pop();
          iter = iter.next();
          continue;
        }
        if (!iter.tuple_next()) {
          throw datapack::SchemaError("Expected TupleNext");
        }
        iter = iter.next();
        break;
      }
      if (parent.list()) {
        assert(!list_stack.empty());
        auto& state = list_stack.top();

        if (state.i != 0) {
          auto keys = state.keys;
          std::size_t key = (*keys)[state.i - 1];
          gui.button("Remove", [=]() { keys.mut().remove(key); });
          gui.end();
        }

        while (state.i < state.keys->size()) {
          gui.key((*state.keys)[state.i]);
          gui.args().horizontal();
          if (gui.group()) {
            break;
          }
          state.i++;
        }

        if (state.i == state.keys->size()) {
          gui.end();
          auto keys_var = state.keys;
          gui.button("new", [keys_var]() { keys_var.mut().append(); });
          gui.end();
          stack.pop();
          list_stack.pop();
          iter = parent.skip();
          continue;
        }

        state.i++;
        iter = parent.next();
        break;
      }
      if (parent.optional()) {
        if (iter != parent.next()) {
          gui.end();
          stack.pop();
          continue;
        }
        break;
      }
      if (parent.variant_next()) {
        if (iter != parent.next()) {
          gui.end();
          stack.pop();

          while (iter != schema.end()) {
            if (iter.variant_end()) {
              iter = iter.next();
              break;
            }
            if (!iter.variant_next()) {
              throw datapack::SchemaError("Expected VariantNext");
            }
            iter = iter.next().skip();
          }
          if (iter == schema.end()) {
            throw datapack::SchemaError("Unexpected end of schema");
          }
          continue;
        }
        break;
      }
      assert(false);
    }
    if (iter == schema.end()) {
      break;
    }

    bool in_object = !stack.empty() && stack.top().object_begin();
    std::size_t n_cells = in_object ? 2 : 1;
    std::string label = next_label;
    next_label.clear();

    if (auto object_begin = iter.object_begin()) {
      if (object_begin->constraint) {
        if (std::get_if<datapack::ConstraintObjectColor>(
                &(*object_begin->constraint))) {
          if (in_object) {
            gui.text_box(label);
          }
          gui.color_picker(Color::Black(), {});
          iter = iter.skip();
          continue;
        }
      }
      gui.args().grid(-1, 2).num_cells(n_cells);
      if (gui.collapsable(label)) {
        stack.push(iter);
        iter = iter.next();
        continue;
      }
      iter = iter.skip();
      continue;
    }
    if (iter.tuple_begin()) {
      gui.args().num_cells(n_cells);
      if (gui.collapsable(label)) {
        stack.push(iter);
        iter = iter.next();
        continue;
      }
      iter = iter.skip();
      continue;
    }
    if (iter.list()) {
      gui.args().num_cells(n_cells);
      if (gui.collapsable(label)) {
        auto keys = gui.variable<KeyList>();
        gui.args().tight();
        if (gui.group()) {
          stack.push(iter);
          list_stack.push({keys, 0});
          iter = iter.next();
          continue;
        }
        gui.button("new", [keys]() { keys.mut().append(); });
        gui.end();
      }
      iter = iter.next().skip();
      continue;
    }
    if (iter.optional()) {
      gui.args().num_cells(n_cells);
      if (gui.collapsable(label)) {
        auto has_value = gui.variable<bool>(false);
        gui.checkbox(has_value);
        if (*has_value) {
          stack.push(iter);
          iter = iter.next();
          next_label = "value";
          continue;
        }
        gui.end();
      }
      iter = iter.next().skip();
      continue;
    }
    if (auto variant_begin = iter.variant_begin()) {
      gui.args().num_cells(n_cells);
      if (gui.collapsable(label)) {
        auto choice = gui.variable<int>(0);
        gui.select(variant_begin->labels, choice);

        iter = iter.next();
        while (iter != schema.end()) {
          if (iter.variant_end()) {
            throw datapack::SchemaError("Didn't find matching VariantNext");
          }
          auto variant_next = iter.variant_next();
          if (!variant_next) {
            throw datapack::SchemaError("Expected VariantNext");
          }
          if (variant_next->index == *choice) {
            break;
          }
          iter = iter.next().skip();
        }
        if (iter == schema.end()) {
          throw datapack::SchemaError("Unexpected end of schema");
        }
        assert(iter.variant_next());
        stack.push(iter);
        iter = iter.next();
        gui.key(*choice);
        next_label = "value";
        continue;
      }
      iter = iter.skip();
      continue;
    }

    if (in_object) {
      gui.text_box(label);
    }

    if (auto number = iter.number()) {
      if (number->constraint) {
        if (auto range = std::get_if<datapack::ConstraintNumberRange>(
                &(*number->constraint))) {
          gui.slider(range->lower, range->lower, range->upper, {});
          iter = iter.next();
          continue;
        }
      }
      switch (number->type) {
      case datapack::NumberType::I32:
        gui.number_input<std::int32_t>(0, {});
        break;
      case datapack::NumberType::I64:
        gui.number_input<std::int64_t>(0, {});
        break;
      case datapack::NumberType::U32:
        gui.number_input<std::uint32_t>(0, {});
        break;
      case datapack::NumberType::U64:
        gui.number_input<std::uint64_t>(0, {});
        break;
      case datapack::NumberType::F32:
        gui.number_input<float>(0, {});
        break;
      case datapack::NumberType::F64:
        gui.number_input<double>(0, {});
        break;
      case datapack::NumberType::U8:
        gui.number_input<std::uint8_t>(0, {});
        break;
      }
      iter = iter.next();
      continue;
    }
    if (iter.string()) {
      gui.text_input("", {});
      iter = iter.next();
      continue;
    }
    if (iter.boolean()) {
      gui.checkbox(false, {});
      iter = iter.next();
      continue;
    }
    if (auto enumerate = iter.enumerate()) {
      gui.select(enumerate->labels, -1, {});
      iter = iter.next();
      continue;
    }
    if (iter.binary()) {
      gui.text_input("", {});
      iter = iter.next();
      continue;
    }
  }
}

} // namespace datagui
