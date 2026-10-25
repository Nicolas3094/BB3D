#ifndef SCLP_JSON_KEYS_HPP_
#define SCLP_JSON_KEYS_HPP_

namespace sclp {

struct JsonKey {
    // shared
    static constexpr auto X = "x";
    static constexpr auto Y = "y";
    static constexpr auto Z = "z";
    static constexpr auto MEASUREMENT = "measurement";
    static constexpr auto CONSTRAINT = "constraint";
    // small item
    static constexpr auto SMALL_ITEMS = "small_items";
    static constexpr auto QUANTITY = "quantity";
    static constexpr auto ORIENTATION = "orientation";
    // large object
    static constexpr auto LARGE_OBJECT = "large_object";
};

} // namespace sclp

#endif
