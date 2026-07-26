#pragma once
#include "bnet/address.hpp"
#include "bnet/error.hpp"
#include "bnet/socket.hpp"
#include <chrono>

namespace bnet {
class Connection {
  public:
	explicit Connection(Socket socket) : m_socket(std::move(socket)) {}

	static auto connect(Address const& address) -> Result<Connection>;

	auto send(std::span<std::byte const> data) -> Result<void>;
	auto receive(std::span<std::byte> buffer) -> Result<std::size_t>;
	auto receive_exact(std::span<std::byte> buffer) -> Result<void>;

	auto send_framed(std::span<std::byte const> data) -> Result<void>;
	auto receive_framed(std::span<std::byte> buffer) -> Result<std::size_t>;

	auto set_no_delay(bool enabled) -> Result<void>;

	auto set_timeout(std::chrono::milliseconds timeout) -> Result<void>;

  private:
	Socket m_socket;
};
} // namespace bnet