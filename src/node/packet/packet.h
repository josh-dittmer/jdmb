#pragma once

#include <memory>
#include <vector>

namespace node {

class Packet {
  public:
    class Header;
    class Data;

    enum class ReadState { HEADER, DATA };

    Packet();
    ~Packet() {}

    std::size_t read(const std::vector<uint8_t>& data, std::size_t offset);

  private:
    std::unique_ptr<Header> m_header;
    std::unique_ptr<Data> m_data;

    ReadState m_read_state;
};

} // namespace node