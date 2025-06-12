#pragma once

#include "../util/result.h"

#include <cstdint>
#include <vector>

namespace stream {

template <typename T, std::size_t Size> class Reader {
  public:
    Reader() {}
    ~Reader() {}

    std::size_t read(const std::vector<uint8_t>& data, std::size_t offset);

    bool is_ready();
    virtual Result<T> consume() = 0;

  private:
    std::size_t m_total_read;
    std::array<uint8_t, Size> m_curr_data;
};

} // namespace stream