#pragma once

#include <assert.h>
#include <cstring>
#include <memory>
#include <vector>

namespace datagui {

template <typename T>
class VectorMap {
public:
  VectorMap() : data(nullptr), data_size(0), data_capacity(0), size_(0) {}

  ~VectorMap() {
    destruct();
  }

  VectorMap(const VectorMap& other) {
    copy_from(other);
  }

  VectorMap(VectorMap&& other) {
    move_from(other);
  }

  VectorMap& operator=(const VectorMap& other) {
    destruct();
    copy_from(other);
    return *this;
  }

  VectorMap& operator=(VectorMap&& other) {
    destruct();
    move_from(other);
    return *this;
  }

  bool contains(int i) const {
    return i >= 0 && i < valid.size() && valid[i];
  }

  const T& operator[](int i) const {
    assert(valid[i]);
    return data[i];
  }

  T& operator[](int i) {
    assert(valid[i]);
    return data[i];
  }

  template <typename... Args>
  int emplace(Args&&... args) {
    int index;
    if (free.empty()) {
      index = data_size;
      expand_data(data_size + 1);
      valid.push_back(false);
    } else {
      index = free.back();
      free.pop_back();
    }
    assert(data_size == valid.size());
    assert(!valid[index]);
    assert(index >= 0 && index < data_size);

    new (&data[index]) T(std::forward<Args>(args)...);
    valid[index] = true;
    size_++;
    return index;
  }

  void pop(int index) {
    assert(index < valid.size() && valid[index]);
    free.push_back(index);
    data[index].~T();
    valid[index] = false;
    assert(size_ > 0);
    size_--;
  }

  std::size_t size() const {
    return size_;
  }

  template <bool Const>
  class Iterator_ {
    using parent_t = std::conditional_t<Const, const VectorMap*, VectorMap*>;

  public:
    T& operator*() const {
      return (*parent)[index];
    }
    T* operator->() const {
      return &(*parent)[index];
    }

    Iterator_& operator++() {
      assert(index < parent->data_size);
      index++;
      while (index < parent->data_size && !parent->valid[index]) {
        index++;
      }
      return *this;
    }
    Iterator_ operator++(int) {
      Iterator_ temp = (*this);
      ++(*this);
      return temp;
    }

    friend bool operator==(const Iterator_& lhs, const Iterator_& rhs) {
      return lhs.index == rhs.index;
    }

  private:
    Iterator_(parent_t parent, int index) : parent(parent), index(index) {
      while (index < parent->data_size && !parent->valid[index]) {
        index++;
      }
    }
    parent_t parent;
    int index;
    friend class VectorMap;
  };
  using ConstIterator = Iterator_<true>;
  using Iterator = Iterator_<false>;

  Iterator begin() {
    std::size_t index = 0;
    while (index < valid.size() && !valid[index]) {
      index++;
    }
    return Iterator(this, index);
  }
  Iterator end() {
    return Iterator(this, data_size);
  }
  Iterator begin() const {
    return const_cast<VectorMap<T>*>(this)->begin();
  }
  Iterator end() const {
    return ConstIterator(this, data_size);
  }
  Iterator cbegin() {
    return ConstIterator(begin());
  }
  Iterator cend() {
    return ConstIterator(this, data_size);
  }

private:
  void expand_data(std::size_t required) {
    assert(required >= data_size);
    if (required <= data_capacity) {
      data_size = required;
      return;
    }
    if (!data) {
      std::size_t new_capacity = 1;
      while (new_capacity < required) {
        new_capacity *= 2;
      }
      data = (T*)malloc(sizeof(T) * new_capacity);
      data_size = required;
      data_capacity = new_capacity;
      return;
    }

    std::size_t new_capacity = data_capacity * 2;
    while (new_capacity < required) {
      new_capacity *= 2;
    }
    T* new_data = (T*)malloc(sizeof(T) * new_capacity);
    move_data(data, new_data, data_size);
    ::free(data);
    data = new_data;
    data_size = required;
    data_capacity = new_capacity;
  }

  void copy_data(const T* from, T* to, std::size_t size) {
    if constexpr (std::is_trivially_copy_constructible_v<T>) {
      memcpy(to, from, sizeof(T) * size);
    }
    if constexpr (!std::is_trivially_copy_constructible_v<T>) {
      for (std::size_t i = 0; i < size; i++) {
        new (to + i) T(from[i]);
      }
    }
  }

  void move_data(T* from, T* to, std::size_t size) {
    if constexpr (std::is_trivially_move_constructible_v<T>) {
      memcpy(to, from, sizeof(T) * size);
    }
    if constexpr (!std::is_trivially_move_constructible_v<T>) {
      for (std::size_t i = 0; i < size; i++) {
        new (to + i) T(std::move(from[i]));
      }
    }
  }

  void destruct_data() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (std::size_t i = 0; i < data_size; i++) {
        data[i].~T();
      }
    }
  }

  void destruct() {
    assert(data_size == valid.size());
    assert(data || (data_size == 0 && data_capacity == 0));
    if (data) {
      destruct_data();
      ::free(data);
      data_size = 0;
      data_capacity = 0;
    }
    valid.clear();
    free.clear();
    size_ = 0;
  }

  void copy_from(const VectorMap& other) {
    if (data) {
      ::free(data);
      data_size = 0;
      data_capacity = 0;
    }
    if (other.data) {
      data_size = other.data_size;
      data_capacity = other.data_capacity;
      data = (T*)malloc(data_capacity);
      copy_data(other.data, data, data_size);
    }

    valid = other.valid;
    free = other.free;
    size_ = other.size_;
  }

  void move_from(VectorMap& other) {
    if (data) {
      ::free(data);
      data_size = 0;
      data_capacity = 0;
    }
    if (other.data) {
      data_size = other.data_size;
      data_capacity = other.data_capacity;
      if constexpr (std::is_trivially_move_constructible_v<T>) {
        data = other.data;
        other.data = nullptr;
      }
      if constexpr (!std::is_trivially_move_constructible_v<T>) {
        data = (T*)malloc(sizeof(data_capacity));
        move_data(other.data, data, data_size);
        other.data = nullptr;
      }
    }

    valid = std::move(other.valid);
    free = std::move(other.free);
    size_ = other.size_;
  }

  T* data;
  std::size_t data_size;
  std::size_t data_capacity;

  std::vector<bool> valid;
  std::vector<int> free;
  std::size_t size_;
};

} // namespace datagui
