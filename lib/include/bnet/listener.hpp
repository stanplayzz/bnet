#pragma once
#include "bnet/connection.hpp"

namespace bnet {
/// @brief A bound, listening TCP socket that accepts incomming connections.
class Listener {
  public:
	/// @brief Wraps an already bound, listening socket.
	explicit Listener(Socket socket) : m_socket(std::move(socket)) {}

	/// @brief Creates a Listener bound to given port on all local interfaces
	/// @param port TCP port to listen on.
	/// @param backlog Maximum length of the queue of pending connections
	/// @return A valid Listener or Error
	[[nodiscard]] static auto create(std::uint16_t port, int backlog = 10) -> Result<Listener>;

	/// @brief Blocks until an incomming connection is accepted or timeout is reached.
	/// @return The accepted connection or Error.
	[[nodiscard]] auto accept() -> Result<Connection>;

	/// @brief Sets send and receive timeout for this connection only.
	/// @param timeout Maximum time to wait per send/receive call.
	auto set_timeout(std::chrono::milliseconds timeout) -> Result<void>;

  private:
	Socket m_socket;
};
} // namespace bnet