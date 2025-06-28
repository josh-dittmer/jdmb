#pragma once

#include "../reader.h"

namespace stream {
namespace packet {

struct Data {
    std::vector<uint8_t> m_data;
};

class DataReader : public Reader<Data, 8192> {
  public:
    static const int MaxDataSize = 8192;

    DataReader(std::size_t size) : Reader("Data"), m_size(size) {}
    ~DataReader() {}

    Result<Data> consume() const override;
    bool should_stop_reading() const override;

  private:
    std::size_t m_size;
};

} // namespace packet
} // namespace stream