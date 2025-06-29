#pragma once

#include "../value_reader.h"

namespace stream {
namespace packet {

struct Data {
    constexpr static const std::size_t MaxSize = 8192;
    std::vector<uint8_t> m_data;
};

class DataReader : public ValueReader<Data, Data::MaxSize> {
  public:
    DataReader(std::size_t size) : ValueReader("Data"), m_size(size) {}
    ~DataReader() {}

    Result<std::shared_ptr<Data>> consume() const override;
    bool should_stop_reading() const override;

  private:
    std::size_t m_size;
};

} // namespace packet
} // namespace stream