#pragma once

#include "../reader.h"

namespace stream {
namespace packet {

struct Header {
    uint32_t m_packet_len;
};

class HeaderReader : public Reader<Header, sizeof(Header)> {
  public:
    HeaderReader() {}
    ~HeaderReader() {}

    Result<Header> consume() override;
};

} // namespace packet
} // namespace stream