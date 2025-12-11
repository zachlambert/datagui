#pragma once

#include <assert.h>
#include <cmath>
#ifdef DATAGUI_DEBUG
#include <iostream>
#endif

namespace datagui {

struct Vec2 {
  union {
    struct {
      float x;
      float y;
    };
    float data[2];
  };

  Vec2() : x(0), y(0) {}
  Vec2(float x, float y) : x(x), y(y) {}
  Vec2(float* data) : data{data[0], data[1]} {}

  float& operator()(std::size_t i) {
    assert(i <= 1);
    return data[i];
  }
  float operator()(std::size_t i) const {
    assert(i <= 1);
    return data[i];
  }

  static Vec2 ones() {
    return Vec2(1, 1);
  }

  static Vec2 uniform(float value) {
    return Vec2(value, value);
  }

  Vec2& operator+=(const Vec2& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  Vec2& operator-=(const Vec2& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  Vec2& operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    return *this;
  }

  Vec2& operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    return *this;
  }

  Vec2& operator*=(const Vec2& rhs) {
    x *= rhs.x;
    y *= rhs.y;
    return *this;
  }

  Vec2& operator/=(const Vec2& rhs) {
    x /= rhs.x;
    y /= rhs.y;
    return *this;
  }

  float length() const {
    return std::hypot(x, y);
  }

  Vec2 abs() const {
    return Vec2(std::abs(x), std::abs(y));
  }

  float dot(const Vec2& other) const {
    return x * other.x + y * other.y;
  }
  float cross(const Vec2& other) const {
    return x * other.y - y * other.x;
  }
};

inline Vec2 operator+(Vec2 lhs, const Vec2& rhs) {
  lhs += rhs;
  return lhs;
}

inline Vec2 operator-(Vec2 lhs, const Vec2& rhs) {
  lhs -= rhs;
  return lhs;
}

inline Vec2 operator*(Vec2 lhs, const Vec2& rhs) {
  lhs *= rhs;
  return lhs;
}

inline Vec2 operator/(Vec2 lhs, const Vec2& rhs) {
  lhs /= rhs;
  return lhs;
}

template <typename T>
inline Vec2 operator*(Vec2 lhs, T rhs) {
  lhs *= rhs;
  return lhs;
}

template <typename T>
inline Vec2 operator*(T lhs, Vec2 rhs) {
  rhs *= lhs;
  return rhs;
}

template <typename T>
inline Vec2 operator/(Vec2 lhs, T rhs) {
  lhs /= rhs;
  return lhs;
}

inline Vec2 operator-(Vec2 op) {
  op.x = -op.x;
  op.y = -op.y;
  return op;
}

inline bool operator==(const Vec2& lhs, const Vec2& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator<=(const Vec2& lhs, const Vec2& rhs) {
  return lhs.x <= rhs.x && lhs.y <= rhs.y;
}

inline bool operator>=(const Vec2& lhs, const Vec2& rhs) {
  return lhs.x >= rhs.x && lhs.y >= rhs.y;
}

inline Vec2 minimum(const Vec2& a, const Vec2& b) {
  return Vec2(std::min(a.x, b.x), std::min(a.y, b.y));
}
inline Vec2 maximum(const Vec2& a, const Vec2& b) {
  return Vec2(std::max(a.x, b.x), std::max(a.y, b.y));
}

struct Vec3 {
  union {
    struct {
      float x;
      float y;
      float z;
    };
    float data[3];
  };

  Vec3() : data{} {}
  Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
  Vec3(float* data) : data{data[0], data[1], data[2]} {}

  float& operator()(std::size_t i) {
    assert(i <= 2);
    return data[i];
  }
  float operator()(std::size_t i) const {
    assert(i <= 2);
    return data[i];
  }

  static Vec3 ones() {
    return Vec3(1, 1, 1);
  }

  static Vec3 uniform(float value) {
    return Vec3(value, value, value);
  }

  Vec3& operator+=(const Vec3& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  Vec3& operator-=(const Vec3& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  Vec3& operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  Vec3& operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }

  float length() const {
    return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
  }

  Vec3 abs() const {
    return Vec3(std::abs(x), std::abs(y), std::abs(z));
  }

  float dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
  }
  Vec3 cross(const Vec3& other) const {
    Vec3 result;
    result.x = y * other.z - z * other.y;
    result.y = z * other.x - x * other.z;
    result.z = x * other.y - y * other.x;
    return result;
  }
};

inline Vec3 operator+(Vec3 lhs, const Vec3& rhs) {
  lhs += rhs;
  return lhs;
}

inline Vec3 operator-(Vec3 lhs, const Vec3& rhs) {
  lhs -= rhs;
  return lhs;
}

template <typename T>
inline Vec3 operator*(Vec3 lhs, T rhs) {
  lhs *= rhs;
  return lhs;
}

template <typename T>
inline Vec3 operator*(T lhs, Vec3 rhs) {
  rhs *= lhs;
  return rhs;
}

template <typename T>
inline Vec3 operator/(Vec3 lhs, T rhs) {
  lhs /= rhs;
  return lhs;
}

inline Vec3 operator-(Vec3 op) {
  op.x = -op.x;
  op.y = -op.y;
  op.z = -op.z;
  return op;
}

#ifdef DATAGUI_DEBUG
std::ostream& operator<<(std::ostream& os, const Vec2& vec);
std::ostream& operator<<(std::ostream& os, const Vec3& vec);
#endif

} // namespace datagui
