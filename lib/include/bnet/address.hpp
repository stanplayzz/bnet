#pragma once
#include <cstdint>
#include <string>

namespace bnet {
/// @brief Network endpoint identified by hostname/IP and port
struct Address {
	std::string host{};
	uint16_t port{};
};
} // namespace bnet