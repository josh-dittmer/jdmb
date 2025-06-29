#pragma once

#include "reader.h"

#include <functional>
#include <type_traits>
#include <vector>

namespace stream {

namespace detail {

template <typename Derived> struct is_derived_from_Reader {
  private:
    template <typename T, std::size_t MaxSize>
    static std::true_type test(const Reader<T, MaxSize>*);

    static std::false_type test(...);

  public:
    static constexpr bool value =
        decltype(test(std::declval<Derived*>()))::value;
};

template <typename... ReaderTs>
struct ReaderConjunction
    : std::conjunction<std::integral_constant<
          bool, is_derived_from_Reader<ReaderTs>::value>...> {};

template <typename T> struct Reader_value_type {
    using type = void;
};

template <typename T, std::size_t MaxSize>
struct Reader_value_type<Reader<T, MaxSize>> {
    using type = T;
};

template <typename Derived> struct extract_Reader_type {
  private:
    template <typename T_, std::size_t Size_>
    static Reader<T_, Size_>* test(Reader<T_, Size_>*);

    static void* test(...);

    using base = decltype(test(std::declval<Derived*>()));

  public:
    using type = typename Reader_value_type<std::remove_pointer_t<base>>::type;
};

template <typename ReaderT>
using ReaderDataT = typename extract_Reader_type<ReaderT>::type;

} // namespace detail

template <typename... ReaderTs> class Sequence {
    static_assert(detail::ReaderConjunction<ReaderTs...>::value,
                  "All Readers must derive from Reader");

    template <typename ReaderT> struct ReaderAndState {
        bool m_enabled = false;
        std::shared_ptr<ReaderT> m_reader;
    };

  public:
    static constexpr std::size_t ReaderCount = sizeof...(ReaderTs);

    template <typename...> struct ReaderCallbackTupleBuilder;

    template <typename C, typename N, typename... Rest>
    struct ReaderCallbackTupleBuilder<C, N, Rest...> {
        using type = decltype(std::tuple_cat(
            std::tuple<
                std::function<std::shared_ptr<N>(const std::shared_ptr<C>&)>>{},
            typename ReaderCallbackTupleBuilder<N, Rest...>::type{}));
    };

    template <typename L> struct ReaderCallbackTupleBuilder<L> {
        using type = std::tuple<std::function<void(const std::shared_ptr<C>&)>>;
    };
    using ReaderCallbackTuple =
        typename ReaderCallbackTupleBuilder<ReaderTs...>::type;

    using ReaderTuple = std::tuple<ReaderAndState<ReaderTs>...>;

    using ErrorCallback = std::function<void(
        const Error& err, const std::shared_ptr<Reader> reader)>;

    using FinishCallback = std::function<void()>;

    template <typename... Args> Sequence(Args... args) : m_curr_offset(0) {
        std::get<0>(m_readers).m_enabled = true;
        std::get<0>(m_readers).m_reader = std::make_shared<
            typename decltype(std::get<0>(m_readers).m_reader)::element_type>(
            args...);
    }

    ~Sequence() {}

    inline void read(const std::vector<uint8_t>& data) {
        perform_reads(std::make_index_sequence<sizeof...(ReaderTs)>{}, data);
    }

    template <std::size_t I>
    inline void on_ready(std::tuple_element_t<I, ReaderCallbackTuple> cb) {
        std::get<I>(m_reader_cbs) = cb;
    }

    inline void on_error(ErrorCallback cb) { m_error_cb = cb; }

    inline void on_finish(FinishCallback cb) { m_finish_cb = cb; }

  private:
    template <std::size_t... Is>
    inline void perform_reads(std::index_sequence<Is...>,
                              const std::vector<uint8_t>& data) {
        (perform_read<Is>(data), ...);
    }

    template <std::size_t I, std::enable_if_t<(I < ReaderCount), int> = 0>
    inline void perform_read(const std::vector<uint8_t>& data) {
        if (read_impl(data)) {
            std::get<I + 1>(m_readers).m_reader =
                std::get<I>(m_reader_cbs)(reader);
        }
    }

    template <std::size_t I, std::enable_if_t<(I == ReaderCount), int> = 0>
    inline void perform_read(const std::vector<uint8_t>& data) {
        if (read_impl(data)) {
            std::get<I>(m_reader_cbs)(reader);
        }
    }

    template <std::size_t I>
    inline bool read_impl(const std::vector<uint8_t>& data) {
        std::shared_ptr reader = std::get<I>(m_readers).m_reader;
        if (reader) {
            if (m_curr_offset >= data.size()) {
                m_curr_offset = 0;
                return false;
            }

            m_curr_offset = reader->read(data, m_curr_offset);
            if (reader->is_ready()) {
                std::get<I>(m_readers).m_reader = nullptr;
                return true;
            }
        }

        return false;
    }

    ReaderCallbackTuple m_reader_cbs;
    ErrorCallback m_error_cb;
    FinishCallback m_finsh_cb;

    ReaderTuple m_readers;

    std::size_t m_curr_offset;
};
} // namespace stream