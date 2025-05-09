#pragma once

#include <assert.h>
#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace datagui {

template <typename E>
class PropIterator;

template <typename E>
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
  template <typename T>
  void push(E id, T&& value) {
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

  template <typename T>
  const T* get(E id) const {
    auto iter = id_offsets.find(id);
    if (iter == id_offsets.end()) {
      return nullptr;
    }
    std::size_t offset = iter->second;

    const auto& header = *((const Header*)(data.data() + offset));
    if (header.type != std::type_index(typeid(T))) {
      return nullptr;
    }
    auto handler_iter = handlers.find(header.type);
    assert(handler_iter != handlers.end());
    auto handler = dynamic_cast<Handler<T>*>(handler_iter->second.get());
    assert(handler);
    return handler->get(data, offset + sizeof(Header));
  }

  void push_checkpoint() {
    checkpoints.push_back(data.size());
  }

  void pop_checkpoint() {
    destruct_from(checkpoints.back());
    checkpoints.pop_back();
  }

  class Iterator;

  PropIterator<E> begin() const;
  PropIterator<E> end() const;

  ~PropStack() {
    destruct_from(0);
  }

private:
  struct Header {
    std::type_index type;
    int id_prev;
    E id; // Assume this is an integer, so most efficient to pack this way
    Header(E id, std::type_index type) : id(id), type(type), id_prev(-1) {}
  };

  void push_header(E id, const std::type_index& type) {
    Header header(id, type);
    auto prev = id_offsets.find(id);
    if (prev != id_offsets.end()) {
      header.id_prev = prev->second;
    }
    id_offsets[id] = data.size();

    data.resize(data.size() + sizeof(Header));
    new ((Header*)(data.data() + data.size() - sizeof(Header)))
        Header(std::move(header));
  }

  void destruct_from(std::size_t start_offset) {
    std::size_t offset = start_offset;
    while (offset < data.size()) {
      auto& header = *((Header*)(data.data() + offset));
      auto iter = handlers.find(header.type);
      assert(iter != handlers.end());
      const auto& handler = *iter->second;

      if (header.id_prev >= 0) {
        id_offsets[header.id] = header.id_prev;
      } else {
        id_offsets.erase(header.id);
      }
      header.~Header();
      offset += sizeof(header);

      handler.destruct(data, offset);
      offset += handler.size();
    }
    assert(offset <= data.size());
    data.resize(start_offset);
  }

  std::unordered_map<std::type_index, std::unique_ptr<HandlerBase>> handlers;
  std::vector<std::uint8_t> data;
  std::vector<std::size_t> checkpoints;
  std::unordered_map<E, std::size_t> id_offsets;

  template <typename E_>
  friend class Prop;
  template <typename E_>
  friend class PropIterator;
};

template <typename E>
class Prop {
  using Header = typename PropStack<E>::Header;

public:
  E id() const {
    return header().id;
  }

  template <typename T>
  const T* as() {
    if (header().type != std::type_index(typeid(T))) {
      return nullptr;
    }

    auto iter = parent->handlers.find(header().type);
    assert(iter != parent->handlers.end());
    const auto* handler =
        dynamic_cast<PropStack<E>::template Handler<T>*>(iter->second.get());
    assert(handler);

    return handler->get(parent->data, offset + sizeof(Header));
  }

private:
  Prop(const PropStack<E>* parent, std::size_t offset) :
      parent(parent), offset(offset) {}
  std::size_t offset;
  const PropStack<E>* parent;

  const Header& header() const {
    return *((Header*)(parent->data.data() + offset));
  }

  friend class PropIterator<E>;
};

// Const only
template <typename E>
class PropIterator {
  using Header = typename PropStack<E>::Header;

public:
  Prop<E> operator*() {
    return Prop(parent, offset);
  }

  // Prefix
  PropIterator& operator++() {
    const auto& header = *((Header*)(parent->data.data() + offset));
    offset += sizeof(Header);

    auto iter = parent->handlers.find(header.type);
    assert(iter != parent->handlers.end());
    const auto& handler = *iter->second;
    offset += handler.size();

    return *this;
  }

  // Postfix
  PropIterator operator++(int) {
    PropIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(const PropIterator& lhs, const PropIterator& rhs) {
    return lhs.parent == rhs.parent && lhs.offset == rhs.offset;
  }
  friend bool operator!=(const PropIterator& lhs, const PropIterator& rhs) {
    return !(lhs == rhs);
  }

private:
  PropIterator(const PropStack<E>* parent, std::size_t offset) :
      parent(parent), offset(offset) {}
  std::size_t offset;
  const PropStack<E>* parent;
  friend class PropStack<E>;
};

template <typename E>
inline PropIterator<E> PropStack<E>::begin() const {
  return PropIterator(this, 0);
}

template <typename E>
inline PropIterator<E> PropStack<E>::end() const {
  return PropIterator(this, data.size());
}

} // namespace datagui
