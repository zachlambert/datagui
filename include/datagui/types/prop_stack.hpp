#pragma once

#include <assert.h>
#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace datagui {

class PropHandler {
  class TypeBase {
  public:
    virtual void copy(const std::uint8_t* from, std::uint8_t* to) = 0;
    virtual void move(std::uint8_t* from, std::uint8_t* to) = 0;
    virtual void destruct(std::uint8_t* ptr) const = 0;
    virtual std::size_t size() const = 0;
  };

  template <typename T>
  class TypeImpl : public TypeBase {
  public:
    void copy(const std::uint8_t* from, std::uint8_t* to) override {
      new ((T*)to) T(*(const T*)to);
    }

    void move(std::uint8_t* from, std::uint8_t* to) override {
      new ((T*)to) T(std::move(*(const T*)to));
    }

    void destruct(std::uint8_t* ptr) const override {
      ((T*)ptr)->~T();
    }

    std::size_t size() const override {
      return sizeof(T);
    }
  };

public:
  template <typename T>
  void require() {
    auto iter = types.find(std::type_index(typeid(T)));
    if (iter == types.end()) {
      types.emplace(
          std::type_index(typeid(T)),
          std::make_shared<TypeImpl<T>>());
    }
  }

  void copy(std::type_index type, const std::uint8_t* from, std::uint8_t* to) {
    types.at(type)->copy(from, to);
  }

  template <typename T>
  void move(std::type_index type, std::uint8_t* from, std::uint8_t* to) {
    types.at(type)->move(from, to);
  }

  void destruct(std::type_index type, std::uint8_t* ptr) const {
    types.at(type)->destruct(ptr);
  }

  std::size_t size(std::type_index type) const {
    return types.at(type)->size();
  }

private:
  std::unordered_map<std::type_index, std::shared_ptr<TypeBase>> types;
};

template <typename K>
struct PropHeader {
  std::type_index type;
  int id_prev;
  K key; // Assume this is an integer, so most efficient to pack this way
  PropHeader(const K& key, std::type_index type) :
      key(key), type(type), id_prev(-1) {}
};

template <typename K>
class PropValue {
  using Header = PropHeader<K>;

public:
  K key() const {
    return header().key;
  }

  template <typename T>
  const T* as() {
    if (header().type != std::type_index(typeid(T))) {
      return nullptr;
    }
    return (const T*)(data->data() + offset + sizeof(Header));
  }

private:
  PropValue(const std::vector<std::uint8_t>* data, std::size_t offset) :
      data(data), offset(offset) {}

  const std::vector<std::uint8_t>* data;
  std::size_t offset;

  const Header& header() const {
    return *((Header*)(data->data() + offset));
  }

  template <typename K_>
  friend class PropIterator;
};

template <typename K>
class PropIterator {
  using Header = PropHeader<K>;

public:
  PropValue<K> operator*() {
    return PropValue<K>(data, offset);
  }

  // Prefix
  PropIterator& operator++() {
    const auto& header = *((Header*)(data->data() + offset));
    offset += sizeof(Header);
    offset += handler->size(header.type);
    return *this;
  }

  // Postfix
  PropIterator operator++(int) {
    PropIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(const PropIterator& lhs, const PropIterator& rhs) {
    return lhs.data == rhs.data && lhs.offset == rhs.offset;
  }
  friend bool operator!=(const PropIterator& lhs, const PropIterator& rhs) {
    return !(lhs == rhs);
  }

private:
  PropIterator(
      const std::vector<std::uint8_t>* data,
      std::size_t offset,
      const PropHandler* handler) :
      data(data), offset(offset), handler(handler) {}

  const std::vector<std::uint8_t>* data;
  std::size_t offset;
  const PropHandler* handler;

  template <typename K_>
  friend class PropContainer;
};

template <typename K>
class PropIterator;

template <typename K>
class PropContainer {
  using Header = PropHeader<K>;

public:
  PropContainer() = default;

  ~PropContainer() {
    clear_from(0);
  }

  PropContainer(const PropContainer& other) : data(), handler(other.handler) {
    std::size_t offset = 0;
    data.resize(other.data.size());
    while (offset < other.data.size()) {
      const auto& header = *((const Header*)(other.data.data() + offset));
      offset += sizeof(Header);
      handler.copy(
          header.type,
          other.data.data() + offset,
          data.data() + offset);
      offset += handler.size(header.type);
    }
  }

  PropContainer(PropContainer&& other) = default;

  PropContainer& operator=(const PropContainer& other) {
    clear_from(0);
    handler = other.handler;

    std::size_t offset = 0;
    data.resize(other.data.size());

    while (offset < other.data.size()) {
      const auto& header = *((const Header*)(other.data.data() + offset));
      offset += sizeof(Header);
      handler.copy(
          header.type,
          other.data.data() + offset,
          data.data() + offset);
      offset += handler.size(header.type);
    }

    return *this;
  }

  PropContainer& operator==(PropContainer&& other) {
    clear_from(0);
    handler = std::move(other.handler);
    data = std::move(other.data);
  }

  void clear_from(std::size_t start_offset) {
    std::size_t offset = start_offset;
    while (offset < data.size()) {
      auto& header = *((Header*)(data.data() + offset));
      std::type_index type = header.type;

      header.~Header();
      offset += sizeof(header);

      handler.destruct(type, data.data() + offset);
      offset += handler.size(type);
    }
    assert(offset <= data.size());
    data.resize(start_offset);
  }

  template <typename T>
  std::size_t push(const K& key, T&& value) {
    std::size_t start_offset = data.size();

    Header header(key, std::type_index(typeid(T)));

    data.resize(data.size() + sizeof(Header));
    new ((Header*)(data.data() + data.size() - sizeof(Header)))
        Header(std::move(header));

    data.resize(data.size() + sizeof(T));
    new (data.data() + data.size() - sizeof(T)) T(std::forward<T>(value));

    handler.require<T>();

    return start_offset;
  }

  template <typename T>
  void replace(std::size_t offset, const K& key, T&& value) {
    assert(offset + sizeof(Header) + sizeof(T) <= data.size());
    const auto& header = *((Header*)(data.data() + offset));
    assert(header.key == key);
    assert(header.type == std::type_index(typeid(T)));
    *((std::decay_t<T>*)(data.data() + offset + sizeof(Header))) =
        std::forward<T>(value);
  }

  template <typename T>
  const T* get(std::size_t offset, const K& key) const {
    const auto& header = *((const Header*)(data.data() + offset));
    if (header.key != key) {
      return nullptr;
    }
    assert(header.type == std::type_index(typeid(T)));
    return (const T*)(data.data() + offset + sizeof(Header));
  }

  std::size_t size() const {
    return data.size();
  }

  PropIterator<K> begin() const {
    return PropIterator<K>(&data, 0, &handler);
  }

  PropIterator<K> end() const {
    return PropIterator<K>(&data, data.size(), &handler);
  }

private:
  std::vector<std::uint8_t> data;
  PropHandler handler;
};

template <typename K>
class PropSet {
public:
  template <typename T>
  void insert(const K& key, T&& value) {
    auto iter = key_offsets.find(key);
    if (iter == key_offsets.end()) {
      std::size_t offset = props.push(key, std::forward<T>(value));
      key_offsets.emplace(key, offset);
    } else {
      props.replace(iter->second, key, value);
    }
  }

  template <typename T>
  const T* get(const K& key) const {
    auto iter = key_offsets.find(key);
    if (iter == key_offsets.end()) {
      return nullptr;
    }
    return props.template get<T>(iter->second, key);
  }

  PropIterator<K> begin() const {
    return props.begin();
  }

  PropIterator<K> end() const {
    return props.end();
  }

private:
  PropContainer<K> props;
  std::unordered_map<K, std::size_t> key_offsets;
};

template <typename K>
class PropStack {
public:
  template <typename T>
  void push(const K& key, T&& value) {
    std::size_t offset = props.push(key, std::forward<T>(value));
    auto iter = key_offsets.find(key);
    if (iter == key_offsets.end()) {
      key_offsets.emplace(key, {offset});
    } else {
      iter->second.push_back(offset);
    }
  }

  template <typename T>
  const T* get(const K& key) const {
    auto iter = key_offsets.find(key);
    if (iter == key_offsets.end()) {
      return nullptr;
    }
    return props.template get<T>(iter->second.back(), key);
  }

  void push_checkpoint() {
    checkpoints.push_back(props.size());
  }

  void pop_checkpoint() {
    for (auto& stack : key_offsets) {
      while (!stack.empty() && stack.top() >= checkpoints.back()) {
        stack.pop_back();
      }
    }
    props.clear_from(checkpoints.back());
    checkpoints.pop_back();
  }

  PropIterator<K> begin() const {
    return props.begin();
  }

  PropIterator<K> end() const {
    return props.end();
  }

private:
  PropContainer<K> props;
  std::unordered_map<K, std::vector<std::size_t>> key_offsets;
  std::vector<std::size_t> checkpoints;
};

} // namespace datagui
