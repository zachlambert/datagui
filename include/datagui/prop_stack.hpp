#pragma once

#include <assert.h>
#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace datagui {

class PropStack {
  class HandlerBase {
  public:
    virtual void destruct(std::vector<std::uint8_t>& data, std::size_t offset)
        const = 0;
    virtual std::size_t size() const = 0;
  };

  template <typename T>
  class Handler : public HandlerBase {
  public:
    void push(std::vector<std::uint8_t>& data, T&& value) const {
      data.resize(data.size() + sizeof(T));
      new ((T*)(data.data() + data.size() - sizeof(T)))
          T{std::forward<T>(value)};
    }

    void destruct(std::vector<std::uint8_t>& data, std::size_t offset)
        const override {
      assert(offset + sizeof(T) <= data.size());
      ((T*)(data.data() + offset))->~T();
    }

    const T* get(const std::vector<std::uint8_t>& data, std::size_t offset)
        const {
      return ((T*)(data.data() + offset));
    }
    std::size_t size() const override {
      return sizeof(T);
    }
  };

public:
  using id_t = int;

  template <typename T>
  void push(id_t id, T&& value) {
    auto iter = handlers.find(std::type_index(typeid(T)));
    if (iter == handlers.end()) {
      iter = handlers
                 .emplace(
                     std::type_index(typeid(T)),
                     std::make_unique<Handler<T>>())
                 .first;
    }
    auto handler = dynamic_cast<Handler<T>*>(iter->second.get());
    assert(handler);
    push_header(id, std::type_index(typeid(T)));
    handler->push(data, std::forward<T>(value));
  }

  void push_checkpoint() {
    checkpoints.push_back(data.size());
  }

  void pop_checkpoint() {
    destruct_from(checkpoints.back());
    checkpoints.pop_back();
  }

  class Iterator;

  class Value {
  public:
    id_t id() const {
      return *((id_t*)(parent->data.data() + offset));
    }
    template <typename T>
    const T* as() {
      const auto& type = *((const std::type_index*)(parent->data.data() +
                                                    offset + sizeof(id_t)));
      if (type != std::type_index(typeid(T))) {
        return nullptr;
      }

      auto iter = parent->handlers.find(type);
      assert(iter != parent->handlers.end());
      const auto* handler = dynamic_cast<Handler<T>*>(iter->second.get());
      assert(handler);

      return handler->get(
          parent->data,
          offset + sizeof(id_t) + sizeof(std::type_index));
    }

  private:
    Value(const PropStack* parent, std::size_t offset) :
        parent(parent), offset(offset) {}
    std::size_t offset;
    const PropStack* parent;

    friend class Iterator;
  };

  // Const only
  class Iterator {
  public:
    Value operator*() {
      return Value(parent, offset);
    }

    // Prefix
    Iterator& operator++() {
      const auto& type = *((const std::type_index*)(parent->data.data() +
                                                    offset + sizeof(id_t)));
      offset += sizeof(id_t) + sizeof(std::type_index);
      auto iter = parent->handlers.find(type);
      assert(iter != parent->handlers.end());
      const auto& handler = *iter->second;
      offset += handler.size();
      return *this;
    }

    // Postfix
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
      return lhs.parent == rhs.parent && lhs.offset == rhs.offset;
    }
    friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
      return !(lhs == rhs);
    }

  private:
    Iterator(const PropStack* parent, std::size_t offset) :
        parent(parent), offset(offset) {}
    std::size_t offset;
    const PropStack* parent;
    friend class PropStack;
  };

  Iterator begin() const {
    return Iterator(this, 0);
  }
  Iterator end() const {
    return Iterator(this, data.size());
  }

  ~PropStack() {
    destruct_from(0);
  }

private:
  void push_header(id_t id, const std::type_index& type) {
    data.resize(data.size() + sizeof(id_t) + sizeof(std::type_index));
    *((id_t*)(data.data() + data.size() - sizeof(id_t) -
              sizeof(std::type_index))) = id;
    new ((std::type_index*)(data.data() + data.size() -
                            sizeof(std::type_index))) std::type_index{type};
  }

  void destruct_header(std::size_t offset) {
    assert(offset + sizeof(id_t) + sizeof(std::type_index) <= data.size());
    // No need to destruct id
    ((std::type_index*)(data.data() + offset + sizeof(id_t)))->~type_index();
  }

  void destruct_from(std::size_t start_offset) {
    std::size_t offset = start_offset;
    while (offset < data.size()) {
      const auto& type =
          *((const std::type_index*)(data.data() + offset + sizeof(id_t)));
      auto iter = handlers.find(type);
      assert(iter != handlers.end());
      const auto& handler = *iter->second;

      destruct_header(offset);
      offset += sizeof(int) + sizeof(std::type_index);

      handler.destruct(data, offset);
      offset += handler.size();
    }
    assert(offset <= data.size());
    data.resize(start_offset);
  }

  std::unordered_map<std::type_index, std::unique_ptr<HandlerBase>> handlers;
  std::vector<std::uint8_t> data;
  std::vector<std::size_t> checkpoints;
};

} // namespace datagui
