#pragma once

#include "../reader.h"

namespace stream {
namespace packet {

struct Header {
    uint32_t m_packet_len;
};

class HeaderReader : public Reader<Header, sizeof(Header)> {
  public:
    HeaderReader() : Reader("Header") {}
    ~HeaderReader() {}

    Result<Header> consume() const override;
    bool should_stop_reading() const override { return false; }
};

} // namespace packet
} // namespace stream