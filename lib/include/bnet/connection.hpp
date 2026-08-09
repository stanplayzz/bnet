#pragma once
#include "bnet/address.hpp"
#include "bnet/error.hpp"
#include "bnet/socket.hpp"
#include <chrono>
#include <utility>

namespace bnet {
/// @brief A bidirectional TCP connection.
///
/// Wraps a Socket and provides raw transport
/// (send/receive) and an optional length-prefixed
/// message protocol (send_framed/receive_framed).
class Connection {
  public:
	/// @brief Wraps an already connected socket.
	explicit Connection(Socket socket, Address address) : m_socket(std::move(socket)), m_address(std::move(address)) {}

	/// @brief Resolves and connects to an endpoint.
	/// @param adress Host and Port to connect to.
	/// @return An established @p Connection or @p Error.
	static auto connect(Address const& address) -> Result<Connection>;

	/// @brief Sends raw bytes.
	/// @param data Payload to send, cannot be empty.
	auto send(std::span<std::byte const> data) -> Result<void>;
	/// @brief Receive whatever is avaible.
	/// @param buffer Destination buffer, cannot be empty.
	/// @return The number of bytes read
	auto receive(std::span<std::byte> buffer) -> Result<std::size_t>;
	/// @brief Receive until @p buffer is completely filled.
	/// @param buffer Destination buffer, cannot be empty.
	auto receive_exact(std::span<std::byte> buffer) -> Result<void>;

	/// @brief Sends @p data as a length-prefixed, versioned message.
	/// @param data Payload to send.
	auto send_framed(std::span<std::byte const> data) -> Result<void>;
	/// @brief Reads exactly one framed message into @p buffer.
	/// @param buffer Destination buffer, must be at least the size of the incomming message.
	/// @return The number of bytes written into @p buffer
	/// @retval Error::ProtocolMismatch The peer's protocol version doesn't match.
	/// @retval Error::InvalidArgument The buffer is smaller than the incomming message.
	auto receive_framed(std::span<std::byte> buffer) -> Result<std::size_t>;

	/// @brief Enables or disables TCP_NODELAY
	/// @param enabled true or false
	auto set_no_delay(bool enabled) -> Result<void>;

	/// @brief Sets send and receive timeout for this connection only.
	/// @param timeout Maximum time to wait per send/receive call.
	auto set_timeout(std::chrono::milliseconds timeout) -> Result<void>;

	/// @brief The remote peer's address.
	[[nodiscard]] auto remote_address() const -> Address const& { return m_address; }

  private:
	Socket m_socket;
	Address m_address{};
};
} // namespace bnet