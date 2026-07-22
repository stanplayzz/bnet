#pragma once
#include <cstdint>
#include <string>

namespace bnet {
struct Address {
	std::string host{};
	uint16_t port{};
};
} // namespace bnet