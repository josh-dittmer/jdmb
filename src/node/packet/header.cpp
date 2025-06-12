#include "header.h"

namespace node {

std::size_t Packet::Header::read(const std::vector<uint8_t>& data,
                                 std::size_t offset) {
    for (std::size_t i = m_curr_read; i + offset < data.size() && i < Size;
         i++) {
        m_curr_data[i] = data[i + offset];
        m_curr_read++;
    }
}

bool Packet::Header::is_ready() { return m_curr_read >= Size; }

Result<Packet::Header::Data> Packet::Header::consume() {
    if (m_curr_read < Size) {
        return Result<Data>::Err(Error(__func__, "partial header"));
    }

    m_curr_read = 0;

    Data data;
    data.m_packet_len = (m_curr_data[0]) | (m_curr_data[1] << 8) |
                        (m_curr_data[2] << 16) | (m_curr_data[3] << 24);

    return Result<Data>::Ok(data);
}

} // namespace node