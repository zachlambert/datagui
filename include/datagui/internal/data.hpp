#pragma once

#include "datagui/internal/tree.hpp"
#include "datagui/internal/vector_map.hpp"
#include <any>
#include <assert.h>

namespace datagui {

struct Data {
  std::any value;
  std::vector<int> nodes;
  Data(const std::any& value) : value(value) {}
};

template <typename T>
class DataPtr {
public:
  const T* operator->() {
    access();
    return &value();
  }
  const T& operator*() {
    access();
    return value();
  }

  T& mut() {
    mutate();
    return value();
  }

  DataPtr& operator=(const T& other) {
    mutate();
    value() = other;
    return *this;
  }

  DataPtr& operator=(T&& other) {
    mutate();
    std::any_cast<T>((*datas)[index].value) = other;
    return *this;
  }

private:
  DataPtr(Tree* tree, VectorMap<Data>* datas, int index) : tree(tree), datas(datas), index(index) {}

  T& value() {
    auto ptr = std::any_cast<T>(&(*datas)[index].value);
    assert(ptr);
    return *ptr;
  }

  void access() {
    auto& data = (*datas)[index];
    for (int existing : data.nodes) {
      if (existing == tree->get_parent()) {
        return;
      }
    }
    data.nodes.push_back(tree->get_parent());
  }

  void mutate() {
    for (int node : (*datas)[index].nodes) {
      tree->queue_changed(node);
    }
  }

  Tree* tree;
  VectorMap<Data>* datas;
  int index;

  friend class DataStore;
};

class DataStore {
public:
  DataStore(Tree* tree) : tree(tree) {}

  template <typename T>
  int create(const T& initial_value) {
    int index = datas.emplace(initial_value);
    return index;
  }

  template <typename T>
  DataPtr<T> get(int index) {
    return DataPtr<T>(tree, &datas, index);
  }

private:
  Tree* tree;
  VectorMap<Data> datas;
};

} // namespace datagui
