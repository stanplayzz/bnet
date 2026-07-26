#include "bnet/connection.hpp"
#include "bnet/context.hpp"
#include <array>
#include <print>
#include <stdexcept>

auto main() -> int {
	auto context = bnet::Context{};

	auto connection = bnet::Connection::connect({.host = "localhost", .port = 5000});
	if (!connection) { throw std::runtime_error{"Failed to create connection"}; }

	auto data = std::as_bytes(std::span{"Hello, World!"});
	if (auto result = connection->send_framed(data); !result) {
		std::println("Failed to send, {}", bnet::to_string_view(result.error()));
		return 1;
	}

	std::array<std::byte, 1024> buffer{};
	if (auto result = connection->receive_framed(buffer); !result) {
		std::println("Failed to receive, {}", bnet::to_string_view(result.error()));
		return 1;
	}

	return 0;
}