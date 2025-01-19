#pragma once

#include <vector>

namespace datagui {

// NOTE:
// Currently requires everything is default-constructable and assumes
// that the memory cost for retaining popped nodes (until overwritten) is
// unimportant.
template <typename T> class VectorMap {
public:
  const T& operator[](std::size_t i) const { return data[i]; }
  T& operator[](std::size_t i) { return data[i]; }
  template <typename... Args> int emplace(Args&&... args) {
    int index = 0;
    if (free.empty()) {
      index = data.size();
      data.emplace_back(std::forward<Args>(args)...);
    } else {
      index = free.back();
      free.pop_back();
      data[index] = T(std::forward<Args>(args)...);
    }
    return index;
  }

  void pop(int index) { free.push_back(index); }

  std::size_t size() const { return data.size() - free.size(); }

private:
  std::vector<T> data;
  std::vector<int> free;
};

} // namespace datagui
