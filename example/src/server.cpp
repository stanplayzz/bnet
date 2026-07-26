#include "bnet/listener.hpp"
#include <array>
#include <print>
#include <stdexcept>

auto main() -> int {
	auto listener = bnet::Listener::create(5000);
	if (!listener) { throw std::runtime_error{"Failed to create listener"}; }

	auto connection = listener->accept();
	if (!connection) { throw std::runtime_error{"Failed to create connection"}; }

	std::array<std::byte, 1024> buffer{};
	auto result = connection->receive_framed(buffer);
	if (!result) {
		std::println("Failed to receive, {}", bnet::to_string_view(result.error()));
		return 1;
	}

	auto data = std::span{buffer}.first(*result);

	if (auto result = connection->send_framed(data); !result) {
		std::println("Failed to send, {}", bnet::to_string_view(result.error()));
		return 1;
	}

	return 0;
}