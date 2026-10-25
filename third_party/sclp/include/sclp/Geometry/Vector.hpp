#ifndef SCLP_GEOMETRY_VECTOR_HPP_
#define SCLP_GEOMETRY_VECTOR_HPP_

#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include <fmt/core.h>

#include <sclp/Types.hpp>

namespace sclp::details {

template <typename Scalar, typename LargeScalar = Scalar>
class Vector {
public:
    // types
    using scalar = Scalar;
    using large_scalar = LargeScalar;
    using reference = scalar &;
    using const_reference = scalar const &;

    // constructor
    Vector() noexcept
        : _x(0)
        , _y(0)
        , _z(0) {}

    Vector(Scalar const & x, Scalar const & y, Scalar const & z) noexcept
        : _x(x)
        , _y(y)
        , _z(z) {}

    Vector(Scalar && x, Scalar && y, Scalar && z) noexcept
        : _x(std::move(x))
        , _y(std::move(y))
        , _z(std::move(z)) {}

    Vector(std::initializer_list<Scalar> list) {
#ifdef SCLP_DEBUG
        if (list.size() != 3) {
            throw std::invalid_argument{"Vector initializer list must have 3 elements"};
        }
#endif
        auto it = list.begin();
        _x = *it++;
        _y = *it++;
        _z = *it;
    }

    // copy and move
    Vector(Vector const &) = default;
    Vector(Vector &&) = default;
    auto operator=(Vector const &) -> Vector & = default;
    auto operator=(Vector &&) -> Vector & = default;

    template <typename T>
    Vector(Vector<T> const & vector) noexcept
        : _x(static_cast<Scalar>(vector.x()))
        , _y(static_cast<Scalar>(vector.y()))
        , _z(static_cast<Scalar>(vector.z())) {}

    // static constructors
    static auto zeros() -> Vector {
        return Vector(0, 0, 0);
    }
    static auto ones() -> Vector {
        return Vector(1, 1, 1);
    }
    static auto inf() -> Vector {
        return Vector(std::numeric_limits<scalar>::max(), std::numeric_limits<scalar>::max(), std::numeric_limits<scalar>::max());
    }

    // cast
    template <typename T>
    auto cast() const noexcept -> Vector<T> {
        return Vector<T>{static_cast<T>(_x), static_cast<T>(_y), static_cast<T>(_z)};
    }

    // accessors
    inline auto x() const noexcept -> Scalar const & { return _x; }
    inline auto y() const noexcept -> Scalar const & { return _y; }
    inline auto z() const noexcept -> Scalar const & { return _z; }

    inline auto x() noexcept -> Scalar & { return _x; }
    inline auto y() noexcept -> Scalar & { return _y; }
    inline auto z() noexcept -> Scalar & { return _z; }

    template <std::size_t I>
    inline auto get() const noexcept -> Scalar const & {
        static_assert(I == 0 || I == 1 || I == 2);
        if constexpr (I == 0) {
            return _x;
        } else if constexpr (I == 1) {
            return _y;
        } else {
            return _z;
        }
    }

    template <std::size_t I>
    inline auto get() noexcept -> Scalar & {
        static_assert(I == 0 || I == 1 || I == 2);
        if constexpr (I == 0) {
            return _x;
        } else if constexpr (I == 1) {
            return _y;
        } else {
            return _z;
        }
    }

    template <Axis A>
    inline auto get() const noexcept -> Scalar const & {
        static_assert(A == Axis::X || A == Axis::Y || A == Axis::Z);
        return this->get<static_cast<std::size_t>(A)>();
    }

    template <Axis A>
    inline auto get() noexcept -> Scalar & {
        static_assert(A == Axis::X || A == Axis::Y || A == Axis::Z);
        return this->get<static_cast<std::size_t>(A)>();
    }

    inline auto get(Axis const & axis) const noexcept -> Scalar const & {
        if (axis == Axis::X) {
            return _x;
        } else if (axis == Axis::Y) {
            return _y;
        } else {
            return _z;
        }
    }

    inline auto get(Axis const & axis) noexcept -> Scalar & {
        if (axis == Axis::X) {
            return _x;
        } else if (axis == Axis::Y) {
            return _y;
        } else {
            return _z;
        }
    }

    // vector operations
    auto prod() const noexcept -> LargeScalar {
        auto p = LargeScalar{1};
        p *= this->x();
        p *= this->y();
        p *= this->z();
        return p;
    }
    auto dot(Vector const & vector) const noexcept -> LargeScalar {
        auto d = LargeScalar{0};
        d += this->x() * vector.x();
        d += this->y() * vector.y();
        d += this->z() * vector.z();
        return d;
    }
    auto squaredNorm() const noexcept -> LargeScalar { return this->dot(*this); }
    auto norm() const -> double { return std::sqrt(this->squaredNorm()); }
    auto cross(Vector const & vector) -> Vector {
        auto x = this->y() * vector.z() - this->z() * vector.y();
        auto y = this->z() * vector.x() - this->x() * vector.z();
        auto z = this->x() * vector.y() - this->y() * vector.x();
        return Vector{x, y, z};
    }
    auto lexicographical() const noexcept -> Vector {
        auto that = *this;
        if (that._y < that._x) std::swap(that._y, that._x);
        if (that._z < that._x) std::swap(that._z, that._x);
        if (that._z < that._y) std::swap(that._z, that._y);
        return that;
    }
    auto abs() const noexcept -> Vector {
        return Vector{
            std::abs(this->x()),
            std::abs(this->y()),
            std::abs(this->z())};
    }

    // assignment operators
    auto lexicographical_inplace() noexcept -> Vector & {
        return *this;
    }
    auto operator+=(Vector const & vector) -> Vector & {
        _x += vector.x();
        _y += vector.y();
        _z += vector.z();
        return *this;
    }
    auto operator+=(Scalar const & scalar) -> Vector & {
        _x += scalar;
        _y += scalar;
        _z += scalar;
        return *this;
    }
    auto operator-=(Vector const & vector) -> Vector & {
        _x -= vector.x();
        _y -= vector.y();
        _z -= vector.z();
        return *this;
    }
    auto operator-=(Scalar const & scalar) -> Vector & {
        _x -= scalar;
        _y -= scalar;
        _z -= scalar;
        return *this;
    }

    // arithmetic operators
    friend auto operator+(Vector const & lhs, Vector const & rhs) -> Vector {
        return Vector{lhs.x() + rhs.x(), lhs.y() + rhs.y(), lhs.z() + rhs.z()};
    }
    friend auto operator+(Vector const & vector, Scalar const & scalar) -> Vector {
        return Vector{vector.x() + scalar, vector.y() + scalar, vector.z() + scalar};
    }
    friend auto operator-(Vector const & lhs, Vector const & rhs) -> Vector {
        return Vector{lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z()};
    }
    friend auto operator-(Vector const & vector, Scalar const & scalar) -> Vector {
        return Vector{vector.x() - scalar, vector.y() - scalar, vector.z() - scalar};
    }
    friend auto operator*(Scalar const & scalar, Vector const & vector) -> Vector {
        return Vector{scalar * vector.x(), scalar * vector.y(), scalar * vector.z()};
    }
    friend auto operator*(Vector const & vector, Scalar const & scalar) -> Vector {
        return scalar * vector;
    }
    friend auto operator/(Vector const & vector, Scalar const & scalar) -> Vector {
        return Vector{vector.x() / scalar, vector.y() / scalar, vector.z() / scalar};
    }
    friend auto operator/(Scalar const & scalar, Vector const & vector) -> Vector {
        return vector / scalar;
    }
    friend auto operator-(Vector const & vector) -> Vector {
        return Vector{-vector.x(), -vector.y(), -vector.z()};
    }

    // comparison operators
    friend auto operator==(Vector const & lhs, Vector const & rhs) -> bool {
        return lhs.x() == rhs.x() && lhs.y() == rhs.y() && lhs.z() == rhs.z();
    }
    friend auto operator!=(Vector const & lhs, Vector const & rhs) -> bool {
        return !(lhs == rhs);
    }
    friend auto operator<(Vector const & lhs, Vector const & rhs) -> bool {
        return (lhs.x() < rhs.x()) && (lhs.y() < rhs.y()) && (lhs.z() < rhs.z());
    }
    friend auto operator<=(Vector const & lhs, Vector const & rhs) -> bool {
        return (lhs.x() <= rhs.x()) && (lhs.y() <= rhs.y()) && (lhs.z() <= rhs.z());
    }
    friend auto operator>(Vector const & lhs, Vector const & rhs) -> bool {
        return (lhs.x() > rhs.x()) && (lhs.y() > rhs.y()) && (lhs.z() > rhs.z());
    }
    friend auto operator>=(Vector const & lhs, Vector const & rhs) -> bool {
        return (lhs.x() >= rhs.x()) && (lhs.y() >= rhs.y()) && (lhs.z() >= rhs.z());
    }
    static auto lexicographical_less(Vector const & lhs, Vector const & rhs) -> bool {
        return std::tie(lhs.x(), lhs.y(), lhs.z()) < std::tie(rhs.x(), rhs.y(), rhs.z());
    }

    // input output
    friend auto to_json(Json & obj, Vector const & vector) -> void {
        obj["x"] = vector.x();
        obj["y"] = vector.y();
        obj["z"] = vector.z();
        return;
    }
    operator std::string() const {
        return Json(*this).dump();
    }
    friend auto operator<<(std::ostream & os, Vector const & vector) -> std::ostream & {
        return os << std::string(vector);
    }

private:
    Scalar _x;
    Scalar _y;
    Scalar _z;
};

} // namespace sclp::details

#endif
