#include "data_reader.h"

namespace stream {
namespace packet {

Result<std::shared_ptr<Data>> DataReader::consume() const {
    if (!stop_condition_reached()) {
        return Result<std::shared_ptr<Data>>::Err(
            Error(__func__, "packet data section too large"));
    }

    std::shared_ptr<Data> data = std::make_shared<Data>();
    data->m_data = std::vector<uint8_t>(m_curr_data.begin(),
                                        m_curr_data.begin() + m_total_read);

    return Result<std::shared_ptr<Data>>::Ok(data);
}

bool DataReader::should_stop_reading() const {
    if (m_total_read >= m_size) {
        return true;
    }

    return false;
}

} // namespace packet
} // namespace stream