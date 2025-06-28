#include "reader.h"

namespace stream {

template <typename T, std::size_t MaxSize>
std::size_t Reader<T, MaxSize>::read(const std::vector<uint8_t>& data,
                                     std::size_t offset) {
    if (m_stop_condition_reached || m_max_size_exceeded) {
        return 0;
    }

    std::size_t read;
    for (read = 0; read + m_total_read + offset < data.size() &&
                   read + m_total_read < MaxSize;
         read++) {
        m_curr_data[read] = data[read + offset];
        m_total_read++;

        if (should_stop_reading()) {
            m_stop_condition_reached = true;
            break;
        }
    }

    if (m_total_read >= MaxSize) {
        m_max_size_exceeded = true;
    }

    return read;
}

} // namespace stream