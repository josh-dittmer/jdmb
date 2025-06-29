#pragma once

#include "../util/result.h"

#include <array>
#include <cstdint>
#include <vector>

namespace stream {

template <typename T> class Reader {
  public:
    Reader(const std::string& name) : m_name(name) {}
    ~Reader() {}

    virtual std::size_t read(const std::vector<uint8_t>& data,
                             std::size_t offset) = 0;
    virtual Result<std::shared_ptr<T>> consume() const = 0;

    virtual bool is_ready() const = 0;

    std::string get_name() const { return m_name; }

  private:
    std::string m_name;
};

} // namespace stream