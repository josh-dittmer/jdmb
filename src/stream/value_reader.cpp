#include "value_reader.h"

#include "packet/data_reader.h"
#include "packet/header_reader.h"

namespace stream {

template <typename T, std::size_t MaxSize>
std::size_t ValueReader<T, MaxSize>::read(const std::vector<uint8_t>& data,
                                          std::size_t offset) {
    if (m_stop_condition_reached || m_max_size_exceeded) {
        return 0;
    }

    std::size_t i;
    for (i = 0;
         i + m_total_read + offset < data.size() && i + m_total_read < MaxSize;
         i++) {
        std::cout << offset << " " << i << " " << m_total_read << std::endl;
        m_curr_data[i] = data[i + m_total_read + offset];

        if (should_stop_reading()) {
            m_stop_condition_reached = true;
            break;
        }
    }

    m_total_read += i;
    if (m_total_read >= MaxSize) {
        m_max_size_exceeded = true;
    }

    return i;
}

template class stream::ValueReader<packet::Data, packet::Data::MaxSize>;

template class stream::ValueReader<packet::Header, sizeof(packet::Header)>;

} // namespace stream