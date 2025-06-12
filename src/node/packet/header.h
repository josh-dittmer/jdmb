#pragma once

#include "../../util/result.h"
#include "packet.h"

#include <array>
#include <cstdint>
#include <vector>

namespace node {

class Packet::Header {
  public:
    static const std::size_t Size = 4;

    struct Data {
        uint32_t m_packet_len;
    };

    Header() {}
    ~Header() {}

    std::size_t read(const std::vector<uint8_t>& data, std::size_t offset);

    bool is_ready();
    Result<Data> consume();

  private:
    std::size_t m_curr_read;
    std::array<uint8_t, Size> m_curr_data;
};

} // namespace node