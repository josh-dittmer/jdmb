#include "header_reader.h"

namespace stream {
namespace packet {

Result<std::shared_ptr<Header>> HeaderReader::consume() const {
    std::shared_ptr<Header> header = std::make_shared<Header>();
    header->m_packet_len = (m_curr_data[0]) | (m_curr_data[1] << 8) |
                           (m_curr_data[2] << 16) | (m_curr_data[3] << 24);

    return Result<std::shared_ptr<Header>>::Ok(header);
}

} // namespace packet
} // namespace stream