#pragma once

#include "detail.h"

#include <functional>
#include <type_traits>
#include <vector>

namespace stream {

namespace sequence_detail {

template <typename ReaderT> struct ReaderAndState {
    using type = ReaderT;
    using data_type = detail::ReaderDataT<ReaderT>;

    bool m_enabled = false;
    std::shared_ptr<ReaderT> m_reader;
};

template <typename...> struct ReaderCallbackTupleBuilder;

template <typename C, typename N, typename... Rest>
struct ReaderCallbackTupleBuilder<C, N, Rest...> {
    using type = decltype(std::tuple_cat(
        std::tuple<std::function<std::shared_ptr<N>(
            const std::shared_ptr<detail::ReaderDataT<C>>&)>>{},
        typename ReaderCallbackTupleBuilder<N, Rest...>::type{}));
};

template <typename L> struct ReaderCallbackTupleBuilder<L> {
    using type = std::tuple<
        std::function<void(const std::shared_ptr<detail::ReaderDataT<L>>&)>>;
};

template <typename... ReaderTs>
using ReaderCallbackTuple =
    typename ReaderCallbackTupleBuilder<ReaderTs...>::type;

template <typename... ReaderTs>
using ReaderTuple = std::tuple<ReaderAndState<ReaderTs>...>;

} // namespace sequence_detail

template <typename... ReaderTs>
class SequenceReader
    : public Reader<typename sequence_detail::ReaderTuple<ReaderTs...>> {
    static_assert(detail::AllDerivedFromReader<ReaderTs...>::value,
                  "All ReaderTs must derive from Reader");

  public:
    static constexpr std::size_t ReaderCount = sizeof...(ReaderTs);

    template <std::size_t I>
    using ReaderCallbackTupleElementT =
        std::tuple_element_t<I,
                             sequence_detail::ReaderCallbackTuple<ReaderTs...>>;

    using ReaderTupleT = typename sequence_detail::ReaderTuple<ReaderTs...>;

    template <std::size_t I>
    using ReaderTupleElementT = std::tuple_element_t<I, ReaderTupleT>;

    using ErrorCallback =
        std::function<void(const std::string& reader_name, const Error& err)>;

    using FinishCallback = std::function<void()>;

    template <typename... Args>
    SequenceReader(const std::string& name, Args... args)
        : Reader<ReaderTupleT>(name),
          m_readers(std::make_shared<ReaderTupleT>()), m_curr_offset(0),
          m_ready(false) {
        std::get<0>(*m_readers).m_enabled = true;
        std::get<0>(*m_readers).m_reader =
            std::make_shared<typename ReaderTupleElementT<0>::type>(args...);
    }

    ~SequenceReader() {}

    inline std::size_t read(const std::vector<uint8_t>& data,
                            std::size_t offset) override {
        m_last_read = 0;
        perform_reads(std::make_index_sequence<ReaderCount>{}, data, offset);
        return m_last_read;
    }

    inline Result<std::shared_ptr<ReaderTupleT>> consume() const override {
        if (!m_ready) {
            return Result<std::shared_ptr<ReaderTupleT>>::Err(
                Error(__func__, "read unfinished"));
        }

        return Result<std::shared_ptr<ReaderTupleT>>::Ok(m_readers);
    }

    inline bool is_ready() const override { return m_ready; }

    template <std::size_t I>
    inline void on_ready(ReaderCallbackTupleElementT<I> cb) {
        std::get<I>(m_reader_cbs) = cb;
    }

    inline void on_error(ErrorCallback cb) { m_error_cb = cb; }

    inline void on_finish(FinishCallback cb) { m_finsh_cb = cb; }

  public:
    template <std::size_t... Is>
    inline void perform_reads(std::index_sequence<Is...>,
                              const std::vector<uint8_t>& data,
                              std::size_t offset) {
        (perform_read<Is>(data, offset), ...);
    }

    template <std::size_t I>
    inline typename std::enable_if_t<(I + 1 < ReaderCount)>
    perform_read(const std::vector<uint8_t>& data, std::size_t offset) {
        std::shared_ptr<typename ReaderTupleElementT<I>::data_type> res =
            read_impl<I>(data, offset);
        if (res) {
            std::cout << "here" << std::endl;
            std::get<I + 1>(*m_readers).m_reader =
                std::get<I>(m_reader_cbs)(res);
        }
    }

    template <std::size_t I>
    inline typename std::enable_if_t<(I + 1 >= ReaderCount)>
    perform_read(const std::vector<uint8_t>& data, std::size_t offset) {
        std::shared_ptr<typename ReaderTupleElementT<I>::data_type> res =
            read_impl<I>(data, offset);
        if (res) {
            m_ready = true;
            std::get<I>(m_reader_cbs)(res);
            m_finsh_cb();
        }
    }

    template <std::size_t I>
    inline std::shared_ptr<typename ReaderTupleElementT<I>::data_type>
    read_impl(const std::vector<uint8_t>& data, std::size_t offset) {
        std::shared_ptr<typename ReaderTupleElementT<I>::type> reader =
            std::get<I>(*m_readers).m_reader;
        std::cout << data.size() << std::endl;
        std::cout << m_curr_offset << std::endl;
        if (reader) {
            if (m_curr_offset >= data.size()) {
                m_curr_offset = 0;
                return nullptr;
            }

            std::size_t bytes_read = reader->read(data, m_curr_offset);

            m_curr_offset = bytes_read;
            m_last_read += bytes_read;

            if (reader->is_ready()) {
                std::get<I>(*m_readers).m_reader = nullptr;

                Result<
                    std::shared_ptr<typename ReaderTupleElementT<I>::data_type>>
                    res = reader->consume();

                if (!res.is_ok()) {
                    if (m_error_cb) {
                        m_error_cb(reader->get_name(), res.unwrap_err());
                    }
                    return nullptr;
                }

                return res.unwrap();
            }
        }

        return nullptr;
    }

    sequence_detail::ReaderCallbackTuple<ReaderTs...> m_reader_cbs;
    ErrorCallback m_error_cb;
    FinishCallback m_finsh_cb;

    std::shared_ptr<ReaderTupleT> m_readers;

    std::size_t m_curr_offset;
    std::size_t m_last_read;

    bool m_ready;
};
} // namespace stream