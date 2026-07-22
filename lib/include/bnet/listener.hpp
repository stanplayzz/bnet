#pragma once
#include "bnet/connection.hpp"

namespace bnet {
class Listener {
  public:
	explicit Listener(Socket socket) : m_socket(std::move(socket)) {}

	[[nodiscard]] static auto create(std::uint16_t port, int backlog = 10) -> Result<Listener>;

	[[nodiscard]] auto accept() -> Result<Connection>;

  private:
	Socket m_socket;
};
} // namespace bnet