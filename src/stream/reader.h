#pragma once

#include "../util/result.h"

#include <array>
#include <cstdint>
#include <vector>

namespace stream {

template <typename T, std::size_t MaxSize> class Reader {
  public:
    Reader(const std::string& name)
        : m_name(name), m_max_size_exceeded(false),
          m_stop_condition_reached(false) {}
    ~Reader() {}

    std::size_t read(const std::vector<uint8_t>& data, std::size_t offset);
    virtual Result<T> consume() const = 0;

    virtual bool should_stop_reading() const = 0;

    std::string get_name() const { return m_name; }

    bool max_size_exceeded() const { return m_max_size_exceeded; }
    bool stop_condition_reached() const { return m_stop_condition_reached; }
    bool is_ready() const {
        return m_max_size_exceeded || m_stop_condition_reached;
    }

  protected:
    std::size_t m_total_read;
    std::array<uint8_t, MaxSize> m_curr_data;

  private:
    std::string m_name;

    bool m_max_size_exceeded;
    bool m_stop_condition_reached;
};

} // namespace stream