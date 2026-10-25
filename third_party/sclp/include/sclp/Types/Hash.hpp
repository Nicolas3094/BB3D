#ifndef SCLP_TYPES_HASH_HPP_
#define SCLP_TYPES_HASH_HPP_

#include <cstddef>
#include <functional>
#include <string>

namespace sclp {

class Hash {
    /**
     * Usage:
     *
     * Constructors from one value:
     *
     *     auto h = Hash(1);
     *     Hash h = 1;
     *     Hash h(1)
     *
     * Constructors from many values:
     *
     *     auto h = Hash(1, 2, 3, 4);
     *     Hash h(1, 2, 3, 4);
     *
     * Combine unique: non-commutative.
     *     Useful for creating unique hashes from values when the order matters.
     *
     *     Hash h(42);
     *     h.combine_unique(10);
     *     h.combine_unique(1);
     *     // 11328463562146737244
     *
     *     Hash h(42);
     *     h.combine_unique(1);
     *     h.combine_unique(10);
     *     // 14475625231256508040
     *
     * Combine commutative: commutative.
     *     Useful for creating unique hashes from values when the order does not matter.
     *
     *     Hash h(42);
     *     h.combine_commutative(10);
     *     h.combine_commutative(1);
     *     // 844468759061056213
     *
     *     Hash h(42);
     *     h.combine_commutative(1);
     *     h.combine_commutative(10);
     *     // 844468759061056213
     *
     *     Hash h(10);
     *     h.combine_commutative(1);
     *     h.combine_commutative(42);
     *     // 844468759061056213
     *
     * Intrinsic value: it is automatically convertible to Value (aka size_t).
     */
public:
    using Value = std::size_t;

private:
    Value _value;

    static constexpr Value DEFAULT_VALUE = 3926486631;
    static constexpr Value DEFAULT_COMBINE_VALUE = 0x9e3779b9;
    static constexpr Value DEFAULT_MIX_VALUE = 0xe9846af9b1a615dull;

public:
    template <typename T>
    Hash(T value) {
        if constexpr (std::is_pointer_v<T>) {
            this->_value = reinterpret_cast<std::size_t>(value);
        } else {
            this->_value = static_cast<Value>(value);
        }
        hash_mix(this->_value);
        return;
    }

    Hash()
        : Hash(DEFAULT_VALUE) {}

    template <typename T, typename... Args>
    Hash(T first, Args... args)
        : Hash(first) {
        // this allows initializations from many values: Hash(1, 2, 3);
        this->combine_unique(Hash(args...));
    }

    operator Value const &() const noexcept {
        return this->_value;
    }

    inline auto combine_unique(Hash const & that) -> void {
        // the subtraction introduces asymmetry
        // the XOR makes it so that two values with the same difference do not yield the same result
        this->_value = (this->_value ^ DEFAULT_COMBINE_VALUE) - that._value;
        hash_mix(this->_value);
        return;
    }

    inline auto combine_commutative(Hash const & that) -> void {
        this->_value ^= that._value;
        return;
    }

    auto to_string() const noexcept -> std::string {
        auto s = std::string();
        s += "Hash(";
        s += std::to_string(this->_value);
        s += ")";
        return s;
    }

    ~Hash() {}

private:
    static inline auto hash_mix(Value & x) -> void {
        // https://jonkagstrom.com/mx3/mx3_rev2.html
        x ^= x >> 32;
        x *= DEFAULT_MIX_VALUE;
        x ^= x >> 32;
        x *= DEFAULT_MIX_VALUE;
        x ^= x >> 28;
        return;
    }
};

} // namespace sclp

template <>
struct std::hash<sclp::Hash> {
    auto operator()(sclp::Hash const & hash) const noexcept -> std::size_t {
        return std::size_t(hash);
    }
};

#endif
