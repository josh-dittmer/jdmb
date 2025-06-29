#pragma once

#include "../reader.h"

namespace stream {
namespace common {

template <typename ReaderT> class Array {
    static_assert(detail::IsDerivedFromReader<ReaderT>::value,
                  "ReaderT must derive from Reader");
};

} // namespace common
} // namespace stream