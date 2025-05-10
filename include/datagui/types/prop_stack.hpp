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
template <typename T>
static constexpr std::size_t aligned_size =
    sizeof(T) % alignment == 0 ? sizeof(T)
                               : alignment * (sizeof(T) / alignment + 1);

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
      new ((T*)to) T(*(const T*)from);
    }

    void move(std::uint8_t* from, std::uint8_t* to) override {
      new ((T*)to) T(std::move(*(const T*)from));
    }

    void destruct(std::uint8_t* ptr) const override {
      ((T*)ptr)->~T();
    }

    std::size_t size() const override {
      return aligned_size<T>;
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
  PropHeader(const K& key, std::type_index type) : key(key), type(type) {}
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
    return (const T*)(data.data() + offset + aligned_size<Header>);
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
    offset += aligned_size<Header>;
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
  PropContainer() : data_(nullptr), size_(0), alloc_size_(0) {}

  ~PropContainer() {
    clear_from(0);
    if (data_) {
      free(data_);
    }
  }

  PropContainer(const PropContainer& other) : handler(other.handler) {
    std::size_t offset = 0;
    resize(other.size_);
    while (offset < other.size_) {
      const auto* header_from = ((const Header*)(other.data_ + offset));
      auto* header_to = ((Header*)(other.data_ + offset));
      new (header_to) Header(*header_from);
      offset += aligned_size<Header>;

      handler.copy(header_from->type, other.data_ + offset, data_ + offset);
      offset += handler.size(header_from->type);
    }
  }

  PropContainer(PropContainer&& other) : handler(std::move(other.handler)) {
    std::size_t offset = 0;
    resize(other.size_);
    while (offset < other.size_) {
      const auto* header_from = ((const Header*)(other.data_ + offset));
      auto* header_to = ((Header*)(other.data_ + offset));
      new (header_to) Header(std::move(*header_from));
      offset += aligned_size<Header>;

      handler.move(header_to->type, other.data_ + offset, data_ + offset);
      offset += handler.size(header_to->type);
    }
  }

  // TODO: Implement if required
  PropContainer& operator=(const PropContainer& other) = delete;
  PropContainer& operator==(PropContainer&& other) = delete;

  void clear_from(std::size_t start_offset) {
    std::size_t offset = start_offset;
    while (offset < size_) {
      auto& header = *((Header*)(data_ + offset));
      std::type_index type = header.type;

      header.~Header();
      offset += aligned_size<Header>;

      handler.destruct(type, data_ + offset);
      offset += handler.size(type);
    }
    assert(offset == size_);
    size_ = start_offset;
  }

  template <typename T>
  std::size_t push(const K& key, T&& value) {
    std::size_t start_offset = size_;

    Header header(key, std::type_index(typeid(T)));

    resize(size_ + aligned_size<Header> + aligned_size<T>);
    new ((Header*)(data_ + size_ - aligned_size<Header> - aligned_size<T>))
        Header(std::move(header));
    new ((T*)(data_ + size_ - aligned_size<T>)) T(std::forward<T>(value));

    handler.require<T>();

    return start_offset;
  }

  std::size_t push(const PropValue<K>& value) {
    std::size_t start_offset = size_;

    const auto& value_header =
        *((const Header*)(value.data.data() + value.offset));
    handler.require(*value.handler, value_header.type);

    std::size_t type_size = handler.size(value_header.type);

    resize(size_ + aligned_size<Header> + type_size);
    new ((Header*)(data_ + size_ - aligned_size<Header> - type_size))
        Header(value_header);
    handler.copy(
        value_header.type,
        value.data.data() + value.offset + aligned_size<Header>,
        data_ + size_ - type_size);

    return start_offset;
  }

  template <typename T>
  void replace(std::size_t offset, const K& key, T&& value) {
    assert(offset + aligned_size<Header> + aligned_size<T> <= size_);
    const auto& header = *((Header*)(data_ + offset));
    assert(header.key == key);
    assert(header.type == std::type_index(typeid(T)));
    *((std::decay_t<T>*)(data_ + offset + aligned_size<Header>)) =
        std::forward<T>(value);
  }

  template <typename T>
  const T* get(std::size_t offset, const K& key) const {
    const auto& header = *((const Header*)(data_ + offset));
    if (header.key != key) {
      return nullptr;
    }
    assert(header.type == std::type_index(typeid(T)));
    return (const T*)(data_ + offset + aligned_size<Header>);
  }

  std::size_t size() const {
    return size_;
  }

  PropIterator<K> begin() const {
    return PropIterator<K>(std::span<std::uint8_t>(data_, size_), 0, &handler);
  }

  PropIterator<K> end() const {
    return PropIterator<K>(
        std::span<const std::uint8_t>(data_, size_),
        size_,
        &handler);
  }

private:
  void resize(std::size_t new_size) {
    if (new_size <= alloc_size_) {
      size_ = new_size;
      return;
    }
    if (!data_) {
      if (new_size % alignment == 0) {
        alloc_size_ = new_size;
      } else {
        alloc_size_ = alignment * (new_size / alignment + 1);
      }
      data_ = (std::uint8_t*)malloc(alloc_size_);
      size_ = new_size;
      return;
    }

    std::size_t new_alloc_size = alloc_size_ * 2;
    while (new_alloc_size < new_size) {
      new_alloc_size *= 2;
    }

    // Cannot just realloc the memory since some data-types aren't trivially
    // relocatable
    std::uint8_t* new_data = (std::uint8_t*)malloc(new_alloc_size);
    std::size_t offset = 0;
    while (offset < size_) {
      const auto* header_from = ((Header*)(data_ + offset));
      auto* header_to = ((Header*)(new_data + offset));
      new (header_to) Header(std::move(*header_from));
      offset += aligned_size<Header>;

      handler.move(header_from->type, data_ + offset, new_data + offset);
      offset += handler.size(header_from->type);
    }
    free(data_);

    data_ = new_data;
    size_ = new_size;
    alloc_size_ = new_alloc_size;
  }

  std::uint8_t* data_;
  std::size_t size_;
  std::size_t alloc_size_;
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
    std::vector<K> to_remove;
    for (auto& pair : key_offsets) {
      auto& stack = pair.second;
      while (!stack.empty() && stack.back() >= checkpoints.back()) {
        stack.pop_back();
        if (stack.empty()) {
          to_remove.push_back(pair.first);
        }
      }
    }
    for (const auto& key : to_remove) {
      key_offsets.erase(key);
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
