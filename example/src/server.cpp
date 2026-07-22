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
	if (auto result = connection->receive(buffer); !result) {
		std::println("Failed to receive");
		return 1;
	}

	if (auto result = connection->send(buffer); !result) {
		std::println("Failed to send");
		return 1;
	}

	return 0;
}