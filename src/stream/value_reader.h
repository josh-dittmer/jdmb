#pragma once

#include "../util/result.h"
#include "reader.h"

#include <array>
#include <cstdint>
#include <vector>

namespace stream {

template <typename T, std::size_t MaxSize>
class ValueReader : public Reader<T> {
  public:
    ValueReader(const std::string& name)
        : Reader<T>(name), m_total_read(0), m_max_size_exceeded(false),
          m_stop_condition_reached(false) {}
    ~ValueReader() {}

    std::size_t read(const std::vector<uint8_t>& data,
                     std::size_t offset) override;
    virtual Result<std::shared_ptr<T>> consume() const override = 0;

    virtual bool should_stop_reading() const = 0;

    bool max_size_exceeded() const { return m_max_size_exceeded; }
    bool stop_condition_reached() const { return m_stop_condition_reached; }
    bool is_ready() const {
        return m_max_size_exceeded || m_stop_condition_reached;
    }

  protected:
    std::size_t m_total_read;
    std::array<uint8_t, MaxSize> m_curr_data;

  private:
    bool m_max_size_exceeded;
    bool m_stop_condition_reached;
};

} // namespace stream