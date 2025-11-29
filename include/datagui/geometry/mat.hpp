#pragma once

#include "datagui/geometry/vec.hpp"
#include <assert.h>
#include <cmath>
#include <cstddef>

namespace datagui {

struct Mat2 {
  float data[4];

  Mat2() : data{} {}
  Mat2(float a, float b, float c, float d) : data{a, b, c, d} {}
  Mat2(float* data) {
    for (std::size_t i = 0; i < 4; i++) {
      this->data[i] = data[i];
    }
  }
  Mat2(const std::initializer_list<float>& list) {
    assert(list.size() == 4);
    std::size_t i = 0;
    for (float value : list) {
      data[i] = value;
      i++;
    }
  }

  float& operator()(std::size_t i, std::size_t j) {
    assert(i < 2 && j < 2);
    return data[j * 2 + i];
  }
  float operator()(std::size_t i, std::size_t j) const {
    assert(i < 2 && j < 2);
    return data[j * 2 + i];
  }

  static Mat2 identity() {
    return Mat2(1, 0, 0, 1);
  }
};

inline Mat2 operator*(const Mat2& lhs, const Mat2& rhs) {
  Mat2 result;
  for (std::size_t i = 0; i < 2; i++) {
    for (std::size_t j = 0; j < 2; j++) {
      for (std::size_t k = 0; k < 2; k++) {
        result(i, j) += lhs(i, k) * rhs(k, j);
      }
    }
  }
  return result;
}

inline Vec2 operator*(const Mat2& lhs, const Vec2& rhs) {
  Vec2 result;
  for (std::size_t i = 0; i < 2; i++) {
    for (std::size_t k = 0; k < 2; k++) {
      result(i) += lhs(i, k) * rhs(k);
    }
  }
  return result;
}

struct Mat3 {
  float data[9];

  Mat3() : data{} {}
  Mat3(float* data) {
    for (std::size_t i = 0; i < 9; i++) {
      this->data[i] = data[i];
    }
  }
  Mat3(const std::initializer_list<float>& list) {
    assert(list.size() == 9);
    std::size_t i = 0;
    for (float value : list) {
      data[i] = value;
      i++;
    }
  }

  float& operator()(std::size_t i, std::size_t j) {
    assert(i < 3 && j < 3);
    return data[j * 3 + i];
  }
  float operator()(std::size_t i, std::size_t j) const {
    assert(i < 3 && j < 3);
    return data[j * 3 + i];
  }

  static Mat3 identity() {
    Mat3 result;
    for (std::size_t i = 0; i < 3; i++) {
      result(i, i) = 1;
    }
    return result;
  }
};

inline Mat3 operator*(const Mat3& lhs, const Mat3& rhs) {
  Mat3 result;
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; j++) {
      for (std::size_t k = 0; k < 3; k++) {
        result(i, j) += lhs(i, k) * rhs(k, j);
      }
    }
  }
  return result;
}

inline Vec3 operator*(const Mat3& lhs, const Vec3& rhs) {
  Vec3 result;
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t k = 0; k < 3; k++) {
      result(i) += lhs(i, k) * rhs(k);
    }
  }
  return result;
}

struct Mat4 {
  float data[16];

  Mat4() : data{} {}
  Mat4(float* data) {
    for (std::size_t i = 0; i < 16; i++) {
      this->data[i] = data[i];
    }
  }
  Mat4(const std::initializer_list<float>& list) {
    assert(list.size() == 16);
    std::size_t i = 0;
    for (float value : list) {
      data[i] = value;
      i++;
    }
  }

  float& operator()(std::size_t i, std::size_t j) {
    assert(i < 4 && j < 4);
    return data[j * 4 + i];
  }
  float operator()(std::size_t i, std::size_t j) const {
    assert(i < 4 && j < 4);
    return data[j * 4 + i];
  }

  static Mat4 identity() {
    Mat4 result;
    for (std::size_t i = 0; i < 4; i++) {
      result(i, i) = 1;
    }
    return result;
  }
};

inline Mat4 operator*(const Mat4& lhs, const Mat4& rhs) {
  Mat4 result;
  for (std::size_t i = 0; i < 4; i++) {
    for (std::size_t j = 0; j < 4; j++) {
      for (std::size_t k = 0; k < 4; k++) {
        result(i, j) += lhs(i, k) * rhs(k, j);
      }
    }
  }
  return result;
}

} // namespace datagui
