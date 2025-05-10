#pragma once

#include <assert.h>
#include <cstdint>
#include <memory>
#include <span>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace datagui {

static constexpr std::size_t alignment = 8;

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
      assert(long(from) % alignment == 0);
      assert(long(to) % alignment == 0);
      new ((T*)to) T(*(const T*)from);
    }

    void move(std::uint8_t* from, std::uint8_t* to) override {
      assert(long(from) % alignment == 0);
      assert(long(to) % alignment == 0);
      new ((T*)to) T(std::move(*(const T*)from));
    }

    void destruct(std::uint8_t* ptr) const override {
      assert(long(ptr) % alignment == 0);
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

  void require(const PropHandler& other, std::type_index type) {
    auto iter = types.find(type);
    if (iter == types.end()) {
      types.emplace(type, other.types.at(type));
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
  K key;
  int padding;
  PropHeader(const K& key, std::type_index type) :
      key(key), type(type), padding(0) {}
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
    return (const T*)(data.data() + offset + sizeof(Header));
  }

private:
  PropValue(
      std::span<const std::uint8_t> data,
      std::size_t offset,
      const PropHandler* handler) :
      data(data), offset(offset), handler(handler) {}

  std::span<const std::uint8_t> data;
  std::size_t offset;
  const PropHandler* handler;

  const Header& header() const {
    return *((Header*)(data.data() + offset));
  }

  template <typename K_>
  friend class PropIterator;
  template <typename K_>
  friend class PropContainer;
};

template <typename K>
class PropIterator {
  using Header = PropHeader<K>;

public:
  PropValue<K> operator*() {
    return PropValue<K>(data, offset, handler);
  }

  // Prefix
  PropIterator& operator++() {
    const auto& header = *((Header*)(data.data() + offset));
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
    return lhs.data.data() == rhs.data.data() && lhs.offset == rhs.offset;
  }
  friend bool operator!=(const PropIterator& lhs, const PropIterator& rhs) {
    return !(lhs == rhs);
  }

private:
  PropIterator(
      std::span<const std::uint8_t> data,
      std::size_t offset,
      const PropHandler* handler) :
      data(data), offset(offset), handler(handler) {}

  std::span<const std::uint8_t> data;
  std::size_t offset;
  const PropHandler* handler;

  template <typename K_>
  friend class PropContainer;
};

template <typename K>
class PropContainer {
  using Header = PropHeader<K>;

public:
  PropContainer() :
      alloc_ptr(nullptr), alloc_size(0), data(nullptr), data_size(0) {}

  ~PropContainer() {
    clear_from(0);
    if (alloc_ptr) {
      free(alloc_ptr);
    }
  }

  void resize(std::size_t new_data_size) {
    // Allocate via std::uint64_t to ensure a 64-bit byte boundary

    if (new_data_size < data_size) {
      // TODO: Shrink the allocated memory?
      data_size = new_data_size;
      return;
    }

    std::size_t required_alloc_size = new_data_size / 8;
    if (new_data_size % 8 != 0) {
      required_alloc_size++;
    }

    std::size_t new_alloc_size = alloc_size == 0 ? 1 : 2 * alloc_size;
    while (new_alloc_size < required_alloc_size) {
      new_alloc_size *= 2;
    }
    printf("Alloc from: %p %zu\n", alloc_ptr, alloc_size);
    alloc_ptr = (std::uint64_t*)realloc((void*)alloc_ptr, new_alloc_size * 8);
    printf("Alloc to: %p %zu\n", alloc_ptr, new_alloc_size);

    data = (std::uint8_t*)alloc_ptr;
    data_size = new_data_size; // May be below new_alloc_size/8
  }

  PropContainer(const PropContainer& other) : handler(other.handler) {
    std::size_t offset = 0;
    resize(other.size());
    while (offset < other.size()) {
      const auto& header = *((const Header*)(other.data + offset));
      offset += sizeof(Header);
      handler.copy(header.type, other.data + offset, data + offset);
      offset += handler.size(header.type);
    }
  }

  PropContainer(PropContainer&& other) :
      alloc_ptr(other.alloc_ptr),
      alloc_size(other.alloc_size),
      data(other.data),
      data_size(other.data_size),
      handler(std::move(other.handler)) {
    other.alloc_ptr = nullptr;
  }

  PropContainer& operator=(const PropContainer& other) {
    clear_from(0);
    handler = other.handler;

    std::size_t offset = 0;
    resize(other.size());

    while (offset < other.size()) {
      const auto& header = *((const Header*)(other.data + offset));
      offset += sizeof(Header);
      handler.copy(header.type, other.data + offset, data + offset);
      offset += handler.size(header.type);
    }

    return *this;
  }

  PropContainer& operator==(PropContainer&& other) {
    clear_from(0);
    handler = std::move(other.handler);
    alloc_ptr = other.alloc_ptr;
    other.alloc_ptr = nullptr;
    alloc_size = other.alloc_size;
    data = other.data;
    data_size = other.data_size;
  }

  void clear_from(std::size_t start_offset) {
    std::size_t offset = start_offset;
    while (offset < data_size) {
      auto& header = *((Header*)(data + offset));
      std::type_index type = header.type;

      header.~Header();
      offset += sizeof(header);

      handler.destruct(type, data + offset);
      offset += handler.size(type);
    }
    assert(offset <= data_size);
    data_size = start_offset;
  }

  template <typename T>
  std::size_t push(const K& key, T&& value) {
    std::size_t start_offset = data_size;

    Header header(key, std::type_index(typeid(T)));

    resize(data_size + sizeof(Header));
    new ((Header*)(data + data_size - sizeof(Header)))
        Header(std::move(header));

    resize(data_size + sizeof(T));
    new ((T*)(data + data_size - sizeof(T))) T(std::forward<T>(value));

    handler.require<T>();

    return start_offset;
  }

  std::size_t push(const PropValue<K>& value) {
    std::size_t start_offset = data_size;

    const auto& value_header =
        *((const Header*)(value.data->data() + value.offset));
    handler.require(*value.handler, value_header.type);

    std::size_t type_size = handler.size(value_header.type);

    resize(data_size + sizeof(Header));
    new ((Header*)(data + data_size - sizeof(Header))) Header(value_header);

    resize(data_size + type_size);
    handler.copy(
        value_header.type,
        value.data->data() + value.offset + sizeof(Header),
        data + data_size - type_size);

    return start_offset;
  }

  template <typename T>
  void replace(std::size_t offset, const K& key, T&& value) {
    assert(offset + sizeof(Header) + sizeof(T) <= data_size);
    const auto& header = *((Header*)(data + offset));
    assert(header.key == key);
    assert(header.type == std::type_index(typeid(T)));
    *((std::decay_t<T>*)(data + offset + sizeof(Header))) =
        std::forward<T>(value);
  }

  template <typename T>
  const T* get(std::size_t offset, const K& key) const {
    const auto& header = *((const Header*)(data + offset));
    if (header.key != key) {
      return nullptr;
    }
    assert(header.type == std::type_index(typeid(T)));
    return (const T*)(data + offset + sizeof(Header));
  }

  std::size_t size() const {
    return data_size;
  }

  PropIterator<K> begin() const {
    return PropIterator<K>(
        std::span<std::uint8_t>(data, data_size),
        0,
        &handler);
  }

  PropIterator<K> end() const {
    return PropIterator<K>(
        std::span<std::uint8_t>(data, data_size),
        data_size,
        &handler);
  }

private:
  std::uint64_t* alloc_ptr;
  std::size_t alloc_size;
  std::uint8_t* data;
  std::size_t data_size;
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
      key_offsets.emplace(key, std::vector<std::size_t>{offset});
    } else {
      iter->second.push_back(offset);
    }
  }

  void push(const PropValue<K>& value) {
    std::size_t offset = props.push(value);
    auto iter = key_offsets.find(value.key());
    if (iter == key_offsets.end()) {
      key_offsets.emplace(value.key(), std::vector<std::size_t>{offset});
    } else {
      iter->second.push_back(offset);
    }
  }

  void push(const PropSet<K>& set) {
    for (auto prop : set) {
      push(prop);
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
    for (auto& pair : key_offsets) {
      auto& stack = pair.second;
      while (!stack.empty() && stack.back() >= checkpoints.back()) {
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
