#pragma once

#include "../sequence_reader.h"
#include "data_reader.h"
#include "header_reader.h"

namespace stream {
namespace packet {

using Sequence = SequenceReader<HeaderReader, DataReader>;

static Sequence create() {
    Sequence p = Sequence("Packet");

    p.on_ready<0>([&](const std::shared_ptr<stream::packet::Header>& header)
                      -> std::shared_ptr<stream::packet::DataReader> {
        std::cout << header->m_packet_len << std::endl;
        return std::make_shared<stream::packet::DataReader>(
            header->m_packet_len);
    });
    p.on_ready<1>(
        [&](const std::shared_ptr<stream::packet::Data>& data) -> void {
            std::cout << "Data size: " << data->m_data.size() << std::endl;
        });
    p.on_error([&](const std::string& reader_name, const Error& err) -> void {
        std::cout << "read error" << std::endl;
    });

    p.on_finish([&]() -> void { std::cout << "finished" << std::endl; });

    return p;
}

} // namespace packet
} // namespace stream
