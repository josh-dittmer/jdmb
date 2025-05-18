#include <netdb.h>
#include <string>
#include <vector>

namespace util {
namespace str {

extern std::string addr_to_str(addrinfo* addr);
extern std::string bytes_to_str(const std::vector<uint8_t> buf);

extern std::vector<uint8_t> str_to_bytes(const std::string& str);

} // namespace str
} // namespace util