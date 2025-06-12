#include "reader.h"

namespace stream {

template <typename T, std::size_t Size>
std::size_t Reader<T, Size>::read(const std::vector<uint8_t>& data,
                                  std::size_t offset) {
    std::size_t read;
    for (read = 0; read + m_total_read + offset < data.size() &&
                   read + m_total_read < Size;
         read++) {
        m_curr_data[i] = data[i + offset];
        m_total_read++;
    }

    return read;
}

template <typename T, std::size_t Size> bool Reader<T, Size>::is_ready() {
    return m_curr_read >= Size;
}

} // namespace stream