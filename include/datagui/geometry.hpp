#pragma once

#include <cmath>

namespace datagui {

template <typename T>
struct Vec {
    T x;
    T y;

    Vec() {}
    Vec(T x, T y): x(x), y(y) {}

    static Vec Zero() {
        return Vec(0, 0);
    }

    static Vec One() {
        return Vec(1, 1);
    }

    static Vec Constant(T value) {
        return Vec(value, value);
    }

    Vec& operator+=(const Vec& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vec& operator-=(const Vec& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vec& operator*=(double rhs) {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    Vec& operator/=(double rhs) {
        x /= rhs;
        y /= rhs;
        return *this;
    }

    T length() const {
        return std::hypot(x, y);
    }

    Vec abs() const {
        return Vec(std::abs(x), std::abs(y));
    }
};

using Veci = Vec<int>;
using Vecf = Vec<float>;
using Vecd = Vec<double>;


template <typename T>
Vec<T> operator+(Vec<T> lhs, const Vec<T>& rhs) {
    lhs += rhs;
    return lhs;
}

template <typename T>
inline Vec<T> operator-(Vec<T> lhs, const Vec<T>& rhs) {
    lhs -= rhs;
    return lhs;
}

template <typename T>
inline Vec<T> operator*(Vec<T> lhs, T rhs) {
    lhs *= rhs;
    return lhs;
}

template <typename T>
inline Vec<T> operator*(T lhs, Vec<T> rhs) {
    rhs *= lhs;
    return rhs;
}

template <typename T>
inline Vec<T> operator/(Vec<T> lhs, T rhs) {
    lhs /= rhs;
    return lhs;
}

template <typename T>
inline Vec<T> operator/(double lhs, Vec<T> rhs) {
    rhs /= lhs;
    return rhs;
}

template <typename T>
inline Vec<T> operator-(Vec<T> op) {
    op.x = -op.x;
    op.y = -op.y;
    return op;
}

template <typename T>
bool operator==(const Vec<T>& lhs, const Vec<T>& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <typename T>
bool operator<=(const Vec<T>& lhs, const Vec<T>& rhs) {
    return lhs.x <= rhs.x && lhs.y <= rhs.y;
}

template <typename T>
bool operator>=(const Vec<T>& lhs, const Vec<T>& rhs) {
    return lhs.x >= rhs.x && lhs.y >= rhs.y;
}

template <typename T>
Vec<T> minimum(const Vec<T>& a, const Vec<T>& b) {
    return Vec<T>(
        std::min(a.x, b.x),
        std::min(a.y, b.y)
    );
}
template <typename T>
Vec<T> maximum(const Vec<T>& a, const Vec<T>& b) {
    return Vec<T>(
        std::max(a.x, b.x),
        std::max(a.y, b.y)
    );
}


template <typename T>
struct Box {
    Vec<T> lower; // inclusive
    Vec<T> upper; // exclusive

    Box() {}
    Box(const Vec<T>& lower, const Vec<T>& upper):
        lower(lower), upper(upper)
    {}

    bool contains(const Vec<T>& point) const {
        return
            point.x >= lower.x && point.x < upper.x
            && point.y >= lower.y && point.y < upper.y;
    }

    Vec<T> center() const {
        return Vec<T>(
            (lower.x + upper.x) / 2,
            (lower.y + upper.y) / 2
        );
    }

    Vec<T> size() const {
        return Vec<T>(
            (upper.x - lower.x),
            (upper.y - lower.y)
        );
    }

    T area() const {
        return (upper.x - lower.x) * (upper.y - lower.y);
    }

    Vec<T> bottom_left() const {
        return lower;
    }
    Vec<T> bottom_right() const {
        return Vec<T>(upper.x, lower.y);
    }
    Vec<T> top_left() const {
        return Vec<T>(lower.x, upper.y);
    }
    Vec<T> top_right() const {
        return upper;
    }
};

using Boxi = Box<int>;
using Boxf = Box<float>;
using Boxd = Box<double>;

template <typename T>
bool intersects(const Box<T>& a, const Box<T>& b) {
    return b.upper >= a.lower && b.lower <= a.upper;
}

template <typename T>
Box<T> intersection(const Box<T>& a, const Box<T>& b) {
    return Box<T>(
        maximum(a.lower, b.lower),
        minimum(a.upper, b.upper)
    );
}

template <typename T>
Box<T> bounding(const Box<T>& a, const Box<T>& b) {
    return Box<T>(
        minimum(a.lower, b.lower),
        maximum(a.upper, b.upper)
    );
}

} // namespace datagui
