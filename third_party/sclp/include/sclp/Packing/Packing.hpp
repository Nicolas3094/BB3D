#ifndef SCLP_PACKING_PACK_HPP_
#define SCLP_PACKING_PACK_HPP_

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/RectangularCuboid.hpp"
#include "sclp/Packing/AllocatedSmallItem.hpp"
#include "sclp/Packing/Input.hpp"

namespace sclp {

class Packing : public RectangularCuboid {
public:
    Packing()
        : RectangularCuboid(DiscreteVector::zeros())
        , _allocated_small_items()
        , _volume(0) {}

    using Sptr = std::shared_ptr<Packing>;

    auto add(
        SmallItem const & small_item,
        DiscreteVector const & initial_position,
        OrientationType const & orientation) -> void {
        auto const & allocated_small_item = this->_allocated_small_items.emplace_back(small_item, initial_position, orientation);
        this->_measurement = DiscreteVector{
            std::max(this->_measurement.x(), allocated_small_item.final_position().x()),
            std::max(this->_measurement.y(), allocated_small_item.final_position().y()),
            std::max(this->_measurement.z(), allocated_small_item.final_position().z()),
        };
        this->_quantity_map[&small_item] += 1;
        this->_valid_quantity = (this->_quantity_map[&small_item] <= small_item.quantity());
        this->_volume = this->measurement().prod();
        this->_allocated_volume += small_item.measurement().prod();
        this->_occupied_volume_ratio = static_cast<decltype(_occupied_volume_ratio)>(this->_allocated_volume) / this->_volume;
        return;
    }

    auto add(Packing::Sptr const & packing, DiscreteVector const & initial_position) -> void {
        if (!packing->is_built()) {
            throw std::runtime_error("one can only add packings that have already been built");
        }
        _packings_to_allocate.push_back({packing, initial_position});
        this->_measurement = DiscreteVector{
            std::max(this->_measurement.x(), packing->measurement().x() + initial_position.x()),
            std::max(this->_measurement.y(), packing->measurement().y() + initial_position.y()),
            std::max(this->_measurement.z(), packing->measurement().z() + initial_position.z()),
        };
        for (auto const & [small_item_prt, quantity] : packing->quantity_map()) {
            this->_quantity_map[small_item_prt] += quantity;
            this->_valid_quantity &= (this->_quantity_map[small_item_prt] <= small_item_prt->quantity());
        }
        this->_volume = this->measurement().prod();
        this->_allocated_volume += packing->allocated_volume();
        this->_occupied_volume_ratio = static_cast<decltype(_occupied_volume_ratio)>(this->_allocated_volume) / this->_volume;
        return;
    }

    // check methods
    auto is_built() const noexcept -> bool {
        return this->_packings_to_allocate.empty();
    }

    auto is_empty() const noexcept -> bool {
        return this->_allocated_small_items.empty() && this->_packings_to_allocate.empty();
    }

    // access methods
    auto allocated_small_items() const -> std::vector<AllocatedSmallItem> const & {
        if (!this->is_built()) {
            throw std::runtime_error("cannot return allocated small items of a non-built packing");
        }
        return this->_allocated_small_items;
    }

    auto packings_to_allocate() const noexcept -> std::vector<std::tuple<Sptr, DiscreteVector>> const & {
        return this->_packings_to_allocate;
    }

    auto quantity_map() const noexcept -> QuantityMap const & { return _quantity_map; }
    auto valid_quantity() const noexcept -> bool const & { return _valid_quantity; }

    auto volume() const noexcept -> Volume { return this->_volume; }
    auto allocated_volume() const noexcept -> Volume const & { return _allocated_volume; }
    auto occupied_volume_ratio() const noexcept -> double const & { return _occupied_volume_ratio; }

    auto hash() const noexcept -> Hash const & { return this->_hash; }

    // update methods
    auto build() -> void {
        for (auto const & [packing, initial_position] : this->_packings_to_allocate) {
            for (auto const & allocated_small_item : packing->allocated_small_items()) {
                this->_allocated_small_items.emplace_back(
                    allocated_small_item.type(),
                    allocated_small_item.initial_position() + initial_position,
                    allocated_small_item.orientation());
            }
        }
        this->_packings_to_allocate.clear();
        this->update_hash();
        return;
    }

    auto update_hash() -> void {
        auto hash = Hash(
            this->measurement().x(),
            this->measurement().y(),
            this->measurement().z());
        for (auto const & [key, value] : this->_quantity_map) {
            hash.combine_unique(key->index());
            hash.combine_unique(value);
        }
        this->_hash = hash;
        return;
    }

    auto to_string() const -> std::string {
        if (!this->is_built()) {
            throw std::runtime_error("cannot output a non-built packing to string");
        }
        auto obj = Json();
        obj["type"] = "output";
        obj["version"] = "0.5.0";
        // large object
        obj["large_object"] = Json();
        obj["large_object"]["measurement"] = Json();
        obj["large_object"]["measurement"]["x"] = this->measurement().x();
        obj["large_object"]["measurement"]["y"] = this->measurement().y();
        obj["large_object"]["measurement"]["z"] = this->measurement().z();
        // small items
        auto small_items = Json::array();
        for (auto const & allocated_small_item : this->allocated_small_items()) {
            auto small_item_obj = Json();
            small_item_obj["measurement"] = Json();
            small_item_obj["measurement"]["x"] = allocated_small_item.measurement().x();
            small_item_obj["measurement"]["y"] = allocated_small_item.measurement().y();
            small_item_obj["measurement"]["z"] = allocated_small_item.measurement().z();
            small_item_obj["position"] = Json();
            small_item_obj["position"]["x"] = allocated_small_item.initial_position().x();
            small_item_obj["position"]["y"] = allocated_small_item.initial_position().y();
            small_item_obj["position"]["z"] = allocated_small_item.initial_position().z();
            small_items.push_back(std::move(small_item_obj));
        }
        obj["small_items"] = std::move(small_items);
        return obj.dump(2, ' ', true);
    }

    friend auto operator<<(std::ostream & os, Packing const & packing) -> std::ostream & {
        return os << packing.to_string();
    }

public:
    std::vector<AllocatedSmallItem> _allocated_small_items;
    std::vector<std::tuple<Sptr, DiscreteVector>> _packings_to_allocate;

    QuantityMap _quantity_map;
    bool _valid_quantity = true;

    Volume _volume;
    Volume _allocated_volume = 0;
    double _occupied_volume_ratio = 0;

    Hash _hash = 0;
};

} // namespace sclp

template <>
struct std::hash<sclp::Packing> {
    auto operator()(sclp::Packing const & packing) const noexcept -> std::size_t {
        return packing.hash();
    }
};

template <>
struct std::equal_to<sclp::Packing> {
    auto operator()(sclp::Packing const & lhs, sclp::Packing const & rhs) const noexcept -> bool {
        return lhs.hash() == rhs.hash();
    }
};

template <>
struct std::hash<sclp::Packing::Sptr> {
    auto operator()(sclp::Packing::Sptr const & packing) const noexcept -> std::size_t {
        return packing->hash();
    }
};

template <>
struct std::equal_to<sclp::Packing::Sptr> {
    auto operator()(sclp::Packing::Sptr const & lhs, sclp::Packing::Sptr const & rhs) const noexcept -> bool {
        return lhs->hash() == rhs->hash();
    }
};

#endif
