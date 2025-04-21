#pragma once

#include <stdexcept>
#include <string>
#include <variant>

class Error {
  public:
    Error(const std::string& msg) : m_msg(msg) {}
    ~Error() {}

    std::string get_msg() { return m_msg; }

  private:
    std::string m_msg;
};

template <typename T, typename E = Error> class Result {
  public:
    static Result Ok(T value) { return Result(std::move(value), true); }
    static Result Err(E error) { return Result(std::move(error), false); }

    ~Result() {}

    bool is_ok() {
        m_checked = true;
        return m_ok;
    }

    T& unwrap() {
        if (!m_checked || !m_ok) {
            throw std::runtime_error("unwrap not allowed");
        }

        return std::get<T>(m_value);
    }

    E& unwrap_err() {
        if (!m_checked || m_ok) {
            throw std::runtime_error("unwrap_err not allowed");
        }

        return std::get<E>(m_value);
    }

  private:
    Result(T value, bool ok)
        : m_value(std::move(value)), m_ok(ok), m_checked(false) {}
    Result(E error, bool ok)
        : m_value(std::move(error)), m_ok(ok), m_checked(false) {}

    void ensure_checked();

    std::variant<T, E> m_value;
    bool m_ok;

    bool m_checked;
};