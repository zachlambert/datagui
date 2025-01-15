#pragma once

#include <stdexcept>

namespace datagui {

class InitializationError : public std::runtime_error {
public:
  InitializationError(const std::string &message)
      : std::runtime_error(message) {}
};

class WindowError : public std::runtime_error {
public:
  WindowError(const std::string &message) : std::runtime_error(message) {}
};

} // namespace datagui
