#pragma once

#include "../sequence.h"
#include "data_reader.h"
#include "header_reader.h"

namespace stream {
namespace packet {

using PacketSequence = Sequence<HeaderReader, DataReader>;

}
} // namespace stream
