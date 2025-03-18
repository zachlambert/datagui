#pragma once

#include "datagui/internal/tree.hpp"
#include "datagui/internal/vector_map.hpp"
#include <any>
#include <assert.h>
#include <memory>

namespace datagui {

template <typename T>
class DataPtr {
public:
  const T* operator->() {
    access();
    return value;
  }
  const T& operator*() {
    access();
    return *value;
  }
  T& mut() {
    mutate();
    return *value;
  }
  operator bool() const {
    return bool(value);
  }

  DataPtr(Tree* tree, T* value, std::vector<int>* nodes) : tree(tree), value(value), nodes(nodes) {}

private:
  void access() {
    for (int existing : *nodes) {
      if (existing == tree->get_parent()) {
        return;
      }
    }
    nodes->push_back(tree->get_parent());
  }

  void mutate() {
    for (int node : *nodes) {
      tree->queue_changed(node);
    }
  }

  Tree* tree;
  T* value;
  std::vector<int>* nodes;
};

template <typename T>
struct Data {
  std::unique_ptr<T> value;
  std::unique_ptr<std::vector<int>> nodes;

  Data(const T& value) :
      value(std::make_unique<T>(value)), nodes(std::make_unique<std::vector<int>>()) {}
  Data(T&& value) :
      value(std::make_unique<T>(value)), nodes(std::make_unique<std::vector<int>>()) {}

  T& operator*() {
    return *value;
  }
  T* operator->() {
    return value.get();
  }
  const T& operator*() const {
    return *value;
  }
  const T* operator->() const {
    return value.get();
  }

  DataPtr<T> to_ptr(Tree* tree) {
    return DataPtr<T>(tree, value.get(), nodes.get());
  }

  // TODO: Clean up
  void mutate(Tree& tree) {
    for (int node : *nodes) {
      tree.queue_changed(node);
    }
  }
};

struct DataAny {
  std::any value;
  std::unique_ptr<std::vector<int>> nodes;

  template <typename T>
  DataAny(const T& value) : value(value), nodes(std::make_unique<std::vector<int>>()) {}
  template <typename T>
  DataAny(T&& value) : value(value), nodes(std::make_unique<std::vector<int>>()) {}

  template <typename T>
  DataPtr<T> to_ptr(Tree* tree) {
    T* ptr = std::any_cast<T>(&value);
    if (!ptr) {
      throw std::bad_any_cast();
    }
    return DataPtr<T>(tree, ptr, nodes.get());
  }

  // TODO: Clean up
  void mutate(Tree& tree) {
    for (int node : *nodes) {
      tree.queue_changed(node);
    }
  }
};

#if 0
template <typename T>
class DataStore {
public:
  DataStore(Tree* tree) : tree(tree) {}

  int create(const T& initial_value) {
    int index = datas.emplace(std::make_unique<Data<T>>(initial_value));
    return index;
  }

  DataPtr<T> get(int index) {
    auto& data = *datas[index];
    return DataPtr<T>(tree, &data.value, &data.nodes);
  }

private:
  Tree* tree;
  VectorMap<std::unique_ptr<Data<T>>> datas;
};

class DataAnyStore {
public:
  DataAnyStore(Tree* tree) : tree(tree) {}

  template <typename T>
  int create(const T& initial_value) {
    int index = datas.emplace(std::make_unique<DataAny>(initial_value));
    return index;
  }

  template <typename T>
  DataPtr<T> get(int index) {
    auto& data = *datas[index];
    T* value = std::any_cast<T>(&data.value);
    assert(value);
    return DataPtr<T>(tree, value, &data.nodes);
  }

private:
  Tree* tree;
  VectorMap<std::unique_ptr<DataAny>> datas;
};
#endif

} // namespace datagui
