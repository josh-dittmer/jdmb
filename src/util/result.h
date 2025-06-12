#pragma once

#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include <netdb.h>

class Error;
typedef std::monostate None;

template <typename T = None, typename E = Error> class Result {
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

class Error {
  public:
    Error(const std::string& src_func, const Error& prev)
        : m_src_func(src_func), m_backtrace(prev.m_backtrace),
          m_msg(prev.m_msg) {
        m_backtrace.push_back(prev.m_src_func);
    }

    template <typename T, typename E>
    Error(const std::string& src_func, Result<T, E>& prev)
        : Error(std::string(src_func), prev.unwrap_err()) {}

    Error(const std::string& src_func, const std::string& msg)
        : m_src_func(src_func), m_msg(msg) {}

    static Error from_errno(const std::string& parent_func,
                            const std::string& setter_func) {
        Error setter_err = Error(setter_func, strerror(errno));
        return Error(parent_func, setter_err);
    }

    static Error from_ai_err(const std::string& parent_func,
                             const std::string& producer_func, int err) {
        Error producer_err = Error(producer_func, gai_strerror(err));
        return Error(parent_func, producer_err);
    }

    ~Error() {}

    std::string str() const {
        std::string str = "\n\t[START CALLSTACK]";
        str += "\n\t\t" + m_src_func;

        int i = m_backtrace.size() - 1;
        for (; i >= 1; i--) {
            str += "\n\t\t\342\224\234 " + m_backtrace[i];
        }

        if (m_backtrace.size() > 0)
            str += "\n\t\t\342\224\224 " + m_backtrace[0];

        str += " -> [" + m_msg + "]";
        str += "\n\t[END CALLSTACK]";

        return str;
    }

    /*std::string get_src_func() { return m_src_func; }
    std::vector<std::string> get_backtrace() { return m_backtrace; }
    std::string get_msg() { return m_msg; }*/

  private:
    std::string m_src_func;
    std::vector<std::string> m_backtrace;

    std::string m_msg;
};

inline std::string operator+(const std::string& lhs, Error rhs) {
    return lhs + rhs.str();
}

inline std::string operator+(const char* lhs, Error rhs) {
    return std::string(lhs) + rhs.str();
}
