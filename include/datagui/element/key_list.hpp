#pragma once

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace datagui {

class KeyList {
public:
  std::size_t append() {
    std::size_t key = next_key++;
    keys.push_back(key);
    return key;
  }

  std::size_t insert(std::size_t pos) {
    std::size_t key = next_key++;
    keys.insert(keys.begin() + pos, key);
    return key;
  }

  void remove(std::size_t key) {
    auto iter = std::find(keys.begin(), keys.end(), key);
    if (iter == keys.end()) {
      throw std::runtime_error("Key doesn't exist, cannot remove");
    }
    keys.erase(iter);
  }

  std::size_t operator[](std::size_t i) const {
    return keys[i];
  }

  std::size_t size() const {
    return keys.size();
  }

  std::vector<std::size_t>::const_iterator begin() const {
    return keys.begin();
  }
  std::vector<std::size_t>::const_iterator end() const {
    return keys.end();
  }

private:
  std::vector<std::size_t> keys;
  static std::size_t next_key;
};

} // namespace datagui
