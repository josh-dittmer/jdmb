#include "sequence.h"

#include "packet/data_reader.h"
#include "packet/header_reader.h"

namespace stream {

/*template <typename... ReaderTs> Sequence<ReaderTs...>::Sequence() {
    init_readers(std::make_index_sequence<sizeof...(ReaderTs)>{});
}*/

/*template <typename... ReaderTs>
void Sequence<ReaderTs...>::read(const std::vector<uint8_t>& data) {}

template <typename... ReaderTs>
template <std::size_t I, typename ReaderT>
void Sequence<ReaderTs...>::on_ready(
    std::function<
        Sequence<ReaderTs...>::NextReaderT<I>(const ReaderDataT<ReaderT>&)>
        cb) {}*/

/*template <typename... ReaderTs>
template <std::size_t... Is>
void Sequence<ReaderTs...>::init_readers(std::index_sequence<Is...>) {
    (create_reader<Is, ReaderTs>(), ...);
}

template <typename... ReaderTs>
template <std::size_t I, typename ReaderT>
void Sequence<ReaderTs...>::create_reader() {
    std::shared_ptr<ReaderT> reader = std::make_shared<ReaderT>();
    std::cout << I << " " << reader->get_name() << std::endl;
    std::get<I>(m_readers) = reader;
}*/

// template class Sequence<packet::HeaderReader, packet::DataReader>;

} // namespace stream