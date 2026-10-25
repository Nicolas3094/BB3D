
#ifndef SCLP_IO_SMALL_ITEM_READER_HPP_
#define SCLP_IO_SMALL_ITEM_READER_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/IO/DiscreteVectorReader.hpp"
#include "sclp/IO/EnabledOrientationsReader.hpp"
#include "sclp/IO/JsonKey.hpp"
#include "sclp/Packing/SmallItem.hpp"

namespace sclp {

struct SmallItemReader {
    SmallItemReader() = delete;

    static auto read_collection(Json const & data) -> std::vector<SmallItem> {
        std::vector<SmallItem> small_items;
        for (auto index = Index{0}; index < data.size(); ++index) {
            const auto & small_item_data = data[index];
            small_items.emplace_back(read(index, small_item_data));
        }
        return small_items;
    }

    static auto read(Index const & index, Json const & data) -> SmallItem {
        return SmallItem(
            index,
            read_quatity(data),
            DiscreteVectorReader::read(data.at(JsonKey::MEASUREMENT)),
            EnabledOrientationsReader::read(data));
    }

    static auto read_quatity(Json const & data) -> Quantity {
        return data.at(JsonKey::QUANTITY).get<Quantity>();
    }
};

} // namespace sclp

#endif
