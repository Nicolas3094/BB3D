#ifndef SCLP_GEOMETRY_VECTOR_2D_HPP_
#define SCLP_GEOMETRY_VECTOR_2D_HPP_

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <sclp/Types.hpp>

namespace sclp::details {

template <typename Scalar, typename LargeScalar = Scalar>
class Vector2D {
public:
    // types
    using scalar = Scalar;
    using large_scalar = LargeScalar;
    using reference = scalar &;
    using const_reference = scalar const &;

    // constructor
    Vector2D() noexcept
        : _x(0)
        , _y(0) {}

    Vector2D(Scalar const & x, Scalar const & y) noexcept
        : _x(x)
        , _y(y) {}

    Vector2D(Scalar && x, Scalar && y) noexcept
        : _x(std::move(x))
        , _y(std::move(y)) {}

    Vector2D(std::initializer_list<Scalar> list) {
#ifdef SCLP_DEBUG
        if (list.size() != 2) {
            throw std::invalid_argument{"Vector2D initializer list must have 2 elements"};
        }
#endif
        auto it = list.begin();
        _x = *it++;
        _y = *it;
    }

    // copy and move
    Vector2D(Vector2D const &) = default;
    Vector2D(Vector2D &&) = default;
    auto operator=(Vector2D const &) -> Vector2D & = default;
    auto operator=(Vector2D &&) -> Vector2D & = default;

    template <typename T>
    Vector2D(Vector2D<T> const & vector) noexcept
        : _x(static_cast<Scalar>(vector.x()))
        , _y(static_cast<Scalar>(vector.y())) {}

    // static constructors
    static auto zeros() -> Vector2D {
        return Vector2D(0, 0);
    }
    static auto ones() -> Vector2D {
        return Vector2D(1, 1);
    }

    // cast
    template <typename T>
    auto cast() const noexcept -> Vector2D<T> {
        return Vector2D<T>{static_cast<T>(_x), static_cast<T>(_y)};
    }

    // accessors
    inline auto x() const noexcept -> Scalar const & { return _x; }
    inline auto y() const noexcept -> Scalar const & { return _y; }

    inline auto x() noexcept -> Scalar & { return _x; }
    inline auto y() noexcept -> Scalar & { return _y; }

    template <std::size_t I>
    inline auto get() const noexcept -> Scalar const & {
        static_assert(I == 0 || I == 1);
        if constexpr (I == 0) {
            return _x;
        } else {
            return _y;
        }
    }

    template <std::size_t I>
    inline auto get() noexcept -> Scalar & {
        static_assert(I == 0 || I == 1);
        if constexpr (I == 0) {
            return _x;
        } else {
            return _y;
        }
    }

    // vector operations
    auto prod() const noexcept -> LargeScalar {
        auto p = LargeScalar{1};
        p *= this->x();
        p *= this->y();
        return p;
    }
    auto dot(Vector2D const & vector) const noexcept -> LargeScalar {
        auto d = LargeScalar{0};
        d += this->x() * vector.x();
        d += this->y() * vector.y();
        return d;
    }
    auto cross(Vector2D const & vector) const noexcept -> LargeScalar {
        auto c = LargeScalar{0};
        c += this->x() * vector.y();
        c -= this->y() * vector.x();
        return c;
    }
    auto squaredNorm() const noexcept -> LargeScalar { return this->dot(*this); }
    auto norm() const -> double { return std::sqrt(this->squaredNorm()); }

    // assignment operators
    auto operator+=(Vector2D const & vector) -> Vector2D & {
        _x += vector.x();
        _y += vector.y();
        return *this;
    }
    auto operator+=(Scalar const & scalar) -> Vector2D & {
        _x += scalar;
        _y += scalar;
        return *this;
    }
    auto operator-=(Vector2D const & vector) -> Vector2D & {
        _x -= vector.x();
        _y -= vector.y();
        return *this;
    }
    auto operator-=(Scalar const & scalar) -> Vector2D & {
        _x -= scalar;
        _y -= scalar;
        return *this;
    }

    // arithmetic operators
    friend auto operator+(Vector2D const & lhs, Vector2D const & rhs) -> Vector2D {
        return Vector2D{lhs.x() + rhs.x(), lhs.y() + rhs.y()};
    }
    friend auto operator+(Vector2D const & vector, Scalar const & scalar) -> Vector2D {
        return Vector2D{vector.x() + scalar, vector.y() + scalar};
    }
    friend auto operator-(Vector2D const & lhs, Vector2D const & rhs) -> Vector2D {
        return Vector2D{lhs.x() - rhs.x(), lhs.y() - rhs.y()};
    }
    friend auto operator-(Vector2D const & vector, Scalar const & scalar) -> Vector2D {
        return Vector2D{vector.x() - scalar, vector.y() - scalar};
    }
    friend auto operator*(Scalar const & scalar, Vector2D const & vector) -> Vector2D {
        return Vector2D{scalar * vector.x(), scalar * vector.y()};
    }
    friend auto operator*(Vector2D const & vector, Scalar const & scalar) -> Vector2D {
        return scalar * vector;
    }
    friend auto operator/(Vector2D const & vector, Scalar const & scalar) -> Vector2D {
        return Vector2D{vector.x() / scalar, vector.y() / scalar};
    }
    friend auto operator/(Scalar const & scalar, Vector2D const & vector) -> Vector2D {
        return vector / scalar;
    }
    friend auto operator-(Vector2D const & vector) -> Vector2D {
        return Vector2D{-vector.x(), -vector.y()};
    }

    // comparison operators
    friend auto operator==(Vector2D const & lhs, Vector2D const & rhs) -> bool {
        return lhs.x() == rhs.x() && lhs.y() == rhs.y();
    }
    friend auto operator!=(Vector2D const & lhs, Vector2D const & rhs) -> bool {
        return !(lhs == rhs);
    }
    friend auto operator<(Vector2D const & lhs, Vector2D const & rhs) -> bool {
        return (lhs.x() < rhs.x()) && (lhs.y() < rhs.y());
    }
    friend auto operator<=(Vector2D const & lhs, Vector2D const & rhs) -> bool {
        return (lhs.x() <= rhs.x()) && (lhs.y() <= rhs.y());
    }
    friend auto operator>(Vector2D const & lhs, Vector2D const & rhs) -> bool {
        return (lhs.x() > rhs.x()) && (lhs.y() > rhs.y());
    }
    friend auto operator>=(Vector2D const & lhs, Vector2D const & rhs) -> bool {
        return (lhs.x() >= rhs.x()) && (lhs.y() >= rhs.y());
    }

    enum class Orientation {
        CLOCKWISE = -1,
        COLLINEAR = 0,
        COUNTERCLOCKWISE = 1
    };

    // convex-hull related methods
    static auto orientation(Vector2D const & pivot, Vector2D const & first, Vector2D const & second) -> Orientation {
        /**
         * imagine you are standing in the plane at point $pivot$ and are facing $first$.
         * You go to $first$ and if $second$ is to your right
         * (then we say the three vectors turn "clockwise"), the sign of the area is negative,
         * otherwise it is positive.
         * If the three points are collinear, the area is zero.
         */
        auto u = first - pivot;
        auto v = second - pivot;
        auto signed_area = u.cross(v);
        if (signed_area < 0) return Orientation::CLOCKWISE;
        if (signed_area > 0) return Orientation::COUNTERCLOCKWISE;
        return Orientation::COLLINEAR;
    }
    static auto clockwise(Vector2D const & pivot, Vector2D const & first, Vector2D const & second) -> bool {
        return orientation(pivot, first, second) == Orientation::CLOCKWISE;
    }
    static auto counterclockwise(Vector2D const & pivot, Vector2D const & first, Vector2D const & second) -> bool {
        return orientation(pivot, first, second) == Orientation::COUNTERCLOCKWISE;
    }
    static auto collinear(Vector2D const & pivot, Vector2D const & first, Vector2D const & second) -> bool {
        return orientation(pivot, first, second) == Orientation::COLLINEAR;
    }
    static double angle(Vector2D const & first, Vector2D const & pivot, Vector2D const & second) {
        // returns angle first-povot-second in radians
        auto u = first - pivot;
        auto v = second - pivot;
        return std::acos(u.dot(v) / std::sqrt(u.squaredNorm() * v.squaredNorm()));
    }
    static void convex_hull(std::vector<Vector2D> & points) {
        // it does not remove points in the same position
        // adapted from https://cp-algorithms.com/geometry/convex-hull.html
        // this method is shared under the CC BY-SA 4.0 license
        if (points.size() <= 1) {
            return;
        }
        stable_sort(
            points.begin(),
            points.end(),
            [](Vector2D lhs, Vector2D rhs) {
                return std::make_pair(lhs.x(), lhs.y()) < std::make_pair(rhs.x(), rhs.y());
            });
        Vector2D p1 = points[0], p2 = points.back();
        std::vector<Vector2D> up, down;
        up.push_back(p1);
        down.push_back(p1);
        for (std::size_t i = 1; i < points.size(); ++i) {
            if (i == points.size() - 1 || clockwise(p1, points[i], p2)) {
                while (up.size() >= 2 && !clockwise(up[up.size() - 2], up[up.size() - 1], points[i])) {
                    up.pop_back();
                }
                up.push_back(points[i]);
            }
            if (i == points.size() - 1 || counterclockwise(p1, points[i], p2)) {
                while (down.size() >= 2 && !counterclockwise(down[down.size() - 2], down[down.size() - 1], points[i])) {
                    down.pop_back();
                }
                down.push_back(points[i]);
            }
        }
        if (up.size() == points.size()) {
            reverse(points.begin(), points.end());
            return;
        }
        points.clear();
        for (int i = 0; i < (int)up.size(); i++) {
            points.push_back(up[i]);
        }
        for (int i = down.size() - 2; i > 0; i--) {
            points.push_back(down[i]);
        }
        std::reverse(points.begin(), points.end()); // change to counterclockwise order
    }
    static auto insidePolygon(Vector2D const & point, std::vector<Vector2D> const & polygon) -> bool {
        // https://github.com/stevenhalim/cpbook-code/blob/master/ch7/polygon.cpp
        // returns true if point is inside or on (vertex/edge) the polygon
        // and false if it is outside of the polygon.
        // Polygon must be in counterclockwise order.
        // All points must be distinct.
        constexpr double EPSILON = 1e-6;
        std::size_t n = polygon.size();
        if (n <= 3) return false; // avoid point or line
        for (std::size_t i = 0; i < n; ++i) {
            // on vertex or edge?
            auto diff = std::fabs((point - polygon[i]).norm() + (polygon[(i + 1) % n] - point).norm() - (polygon[(i + 1) % n] - polygon[i]).norm());
            if (diff < EPSILON) {
                return true;
            }
        }
        double sum = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            if (counterclockwise(point, polygon[i], polygon[(i + 1) % n]))
                sum += angle(polygon[i], point, polygon[(i + 1) % n]); // left turn / counter clockwise
            else
                sum -= angle(polygon[i], point, polygon[(i + 1) % n]); // right turn / clockwise
        }
        // 360d->in, 0d->out
        return std::fabs(sum) > M_PI;
    }

    // input output
    friend auto to_json(Json & obj, Vector2D const & vector) -> void {
        obj["x"] = vector.x();
        obj["y"] = vector.y();
        return;
    }
    operator std::string() const {
        return Json(*this).dump();
    }
    friend auto operator<<(std::ostream & os, Vector2D const & vector) -> std::ostream & {
        return os << std::string(vector);
    }

private:
    Scalar _x;
    Scalar _y;
};

} // namespace sclp::details

#endif
