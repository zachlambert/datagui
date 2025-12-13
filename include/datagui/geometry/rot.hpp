#pragma once

#include "datagui/geometry/mat.hpp"

namespace datagui {

struct Quat {
  union {
    struct {
      float x;
      float y;
      float z;
      float w;
    };
    float data[4];
  };
  Quat() : data{0, 0, 0, 1} {}
  Quat(float x, float y, float z, float w) : data{x, y, z, w} {}
  Quat(float* data) {
    for (std::size_t i = 0; i < 4; i++) {
      this->data[i] = data[i];
    }
  }

  float operator()(std::size_t i) const {
    return data[i];
  }
  float& operator()(std::size_t i) {
    return data[i];
  }
};

struct Euler {
  union {
    struct {
      float roll;
      float pitch;
      float yaw;
    };
    float data[3];
  };
  Euler() : data{0, 0, 0} {}
  Euler(float roll, float pitch, float yaw) : data{roll, pitch, yaw} {}
  Euler(float* data) {
    for (std::size_t i = 0; i < 3; i++) {
      this->data[i] = data[i];
    }
  }

  float operator()(std::size_t i) const {
    return data[i];
  }
  float& operator()(std::size_t i) {
    return data[i];
  }
};

class Rot2 {
public:
  Rot2() : mat_(Mat2::identity()) {}
  Rot2(float angle) :
      mat_(
          std::cos(angle),
          -std::sin(angle),
          std::sin(angle),
          std::cos(angle)) {}
  Rot2(float* data) : mat_(data) {}
  Rot2(const Mat2& mat) : mat_(mat) {}

  const Mat2& mat() const {
    return mat_;
  }

  static Rot2 line_rot(const Vec2& start, const Vec2& end);

private:
  Mat2 mat_;
};

class Rot3 {
public:
  Rot3() : mat_(Mat3::identity()) {}
  Rot3(float* data) : mat_(data) {}
  Rot3(const Mat3& mat) : mat_(mat) {}

  Rot3(const Euler& euler);
  Rot3(const Quat& quat);

  const Mat3& mat() const {
    return mat_;
  }

  static Rot3 line_rot(const Vec3& start, const Vec3& end);
  static Rot3 line_rot(const Vec3& direction);

private:
  Mat3 mat_;
};

} // namespace datagui
