#include "bnet/connection.hpp"
#include <array>
#include <print>
#include <stdexcept>

auto main() -> int {
	auto connection = bnet::Connection::connect({.host = "localhost", .port = 5000});
	if (!connection) { throw std::runtime_error{"Failed to create connection"}; }

	auto data = std::as_bytes(std::span{"Hello, World!"});
	if (auto result = connection->send(data); !result) {
		std::println("Failed to send");
		return 1;
	}

	std::array<std::byte, 5> buffer;
	if (auto result = connection->receive(buffer); !result) {
		std::println("Failed to receive");
		return 1;
	}

	return 0;
}