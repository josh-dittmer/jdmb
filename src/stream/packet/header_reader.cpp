#include "header_reader.h"

namespace stream {
namespace packet {

Result<Header> HeaderReader::consume() const {
    Header header;
    header.m_packet_len = (m_curr_data[0]) | (m_curr_data[1] << 8) |
                          (m_curr_data[2] << 16) | (m_curr_data[3] << 24);

    return Result<Header>::Ok(header);
}

} // namespace packet
} // namespace stream