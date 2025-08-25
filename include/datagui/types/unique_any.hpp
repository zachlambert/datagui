#pragma once

#include <memory>

namespace datagui {

class UniqueAny {
  class ValueBase {
  public:
    virtual ~ValueBase() {}
  };

  template <typename T>
  class Value : public ValueBase {
  public:
    Value(T&& value) : value(std::move(value)) {}
    T value;
  };

  class ValueNull : public ValueBase {};

public:
  UniqueAny() : value(std::make_unique<ValueNull>()) {}
  UniqueAny(UniqueAny&& other) = default;
  UniqueAny& operator=(UniqueAny&& other) = default;

  template <typename T, typename... Args>
  static UniqueAny Make(Args&&... args) {
    return UniqueAny(T(std::forward<Args>(args)...));
  }

  template <typename T>
  T* cast() {
    if (auto ptr = dynamic_cast<Value<T>*>(value.get())) {
      return &ptr->value;
    }
    return nullptr;
  }

  template <typename T>
  const T* cast() const {
    if (auto ptr = dynamic_cast<const Value<T>*>(value.get())) {
      return &ptr->value;
    }
    return nullptr;
  }

  operator bool() const {
    return !dynamic_cast<const ValueNull*>(value.get());
  }

private:
  template <typename T>
  UniqueAny(T&& value) : value(std::make_unique<Value<T>>(std::move(value))) {}

  std::unique_ptr<ValueBase> value;
};

} // namespace datagui
