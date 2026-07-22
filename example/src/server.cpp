#include "bnet/listener.hpp"
#include <stdexcept>

auto main() -> int {
	auto listener = bnet::Listener::create(5000);
	if (!listener) { throw std::runtime_error{"Failed to create listener"}; }

	auto connection = listener->accept();
	if (!connection) { throw std::runtime_error{"Failed to create connection"}; }

	std::array<std::byte, 1024> buffer{};
	connection->receive(buffer);

	connection->send(buffer);

	return 0;
}