#include "bnet/connection.hpp"
#include <stdexcept>

auto main() -> int {
	auto connection = bnet::Connection::connect({.host = "localhost", .port = 5000});
	if (!connection) { throw std::runtime_error{"Failed to create connection"}; }

	auto data = std::as_bytes(std::span{"Hello, World!"});
	connection->send(data);

	std::array<std::byte, 5> reply;
	connection->receive_exact(reply);

	return 0;
}