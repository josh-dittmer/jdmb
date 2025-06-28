#include "data_reader.h"

namespace stream {
namespace packet {

Result<Data> DataReader::consume() const {
    if (!stop_condition_reached()) {
        return Result<Data>::Err(
            Error(__func__, "packet data section too large"));
    }

    Data data;
    data.m_data = std::vector<uint8_t>(m_curr_data.begin(),
                                       m_curr_data.begin() + m_total_read);

    return Result<Data>::Ok(Data());
}

bool DataReader::should_stop_reading() const {
    if (m_total_read >= m_size) {
        return true;
    }

    return false;
}

} // namespace packet
} // namespace stream