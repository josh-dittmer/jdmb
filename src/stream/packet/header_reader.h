#pragma once

#include "../value_reader.h"

namespace stream {
namespace packet {

struct Header {
    uint32_t m_packet_len;
};

class HeaderReader : public ValueReader<Header, sizeof(Header)> {
  public:
    HeaderReader() : ValueReader("Header") {}
    ~HeaderReader() {}

    Result<std::shared_ptr<Header>> consume() const override;
    bool should_stop_reading() const override { return false; }
};

} // namespace packet
} // namespace stream