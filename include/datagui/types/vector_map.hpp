#pragma once

#include <assert.h>
#include <vector>

namespace datagui {

// NOTE:
// Currently requires everything is default-constructable and assumes
// that the memory cost for retaining popped nodes (until overwritten) is
// unimportant.
template <typename T>
class VectorMap {
public:
  VectorMap() : size_(0) {}

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

  bool contains(std::size_t i) const {
    return i < valid.size() && valid[i];
  }

  const T& operator[](std::size_t i) const {
    assert(valid[i]);
    return (const T&)data[i];
  }

  T& operator[](std::size_t i) {
    assert(valid[i]);
    return (T&)data[i];
  }

  template <typename... Args>
  int emplace(Args&&... args) {
    int index;
    if (free.empty()) {
      index = data.size();
      data.emplace_back();
      valid.push_back(false);
    } else {
      index = free.back();
      free.pop_back();
    }

    assert(!valid[index]);
    new ((T*)&data[index]) T(std::forward<Args>(args)...);
    valid[index] = true;
    size_++;
    return index;
  }

  void pop(int index) {
    free.push_back(index);
    ((T&)data[index]).~T();
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
      assert(index < parent->data.size());
      index++;
      while (index < parent->data.size() && !parent->valid[index]) {
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
    Iterator_(parent_t parent, std::size_t index) :
        parent(parent), index(index) {
      while (index < parent->data.size() && !parent->valid[index]) {
        index++;
      }
    }
    parent_t parent;
    std::size_t index;
    friend class VectorMap;
  };
  using ConstIterator = Iterator_<true>;
  using Iterator = Iterator_<false>;

  Iterator begin() {
    return Iterator(this, 0);
  }
  Iterator end() {
    return Iterator(this, data.size());
  }
  Iterator begin() const {
    return ConstIterator(this, 0);
  }
  Iterator end() const {
    return ConstIterator(this, data.size());
  }
  Iterator cbegin() {
    return ConstIterator(this, 0);
  }
  Iterator cend() {
    return ConstIterator(this, data.size());
  }

private:
  void destruct() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (std::size_t i = 0; i < data.size(); i++) {
        if (valid[i]) {
          valid[i] = false;
          ((T&)data[i]).~T();
        }
      }
    }
  }
  void copy_from(const VectorMap& other) {
    valid = other.valid;
    free = other.free;
    size_ = other.size_;
    if constexpr (std::is_trivially_constructible_v<T>) {
      data = other.data;
    }
    if constexpr (!std::is_trivially_constructible_v<T>) {
      data.resize(other.data.size());
      for (std::size_t i = 0; i < data.size(); i++) {
        if (other->valid[i]) {
          new (&data[i]) T(other[i]);
        }
      }
    }
  }
  void move_from(const VectorMap& other) {
    valid = std::move(other.valid);
    free = std::move(other.free);
    size_ = other.size_;
    if (std::is_trivially_move_constructible_v<T>) {
      data = std::move(other.data);
    }
    if (!std::is_trivially_move_constructible_v<T>) {
      data.resize(other.data.size());
      for (std::size_t i = 0; i < data.size(); i++) {
        if (other->valid[i]) {
          new (&data[i]) T(std::move(other[i]));
          other->valid[i] = false;
        }
      }
    }
  }

  struct TBytes {
    char dummy[sizeof(T)];
  };
  std::vector<TBytes> data;
  std::vector<bool> valid;
  std::vector<int> free;
  std::size_t size_;
};

} // namespace datagui
