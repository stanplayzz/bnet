#pragma once
#include "bnet/address.hpp"
#include "bnet/error.hpp"
#include "bnet/socket_handle.hpp"
#include <chrono>
#include <span>

namespace bnet {
/// @brief Wrapper over a raw OS socket handle
class Socket {
  public:
	explicit Socket(platform::SocketHandle fd) : m_fd(fd) {}

	Socket(Socket const&) = delete;
	Socket& operator=(Socket const&) = delete;
	Socket(Socket&& rhs) noexcept;
	auto operator=(Socket&& rhs) noexcept -> Socket&;

	~Socket() noexcept;

	[[nodiscard]] auto fd() const noexcept -> platform::SocketHandle { return m_fd; }

  private:
	platform::SocketHandle m_fd{static_cast<platform::SocketHandle>(-1)};
};

/// @brief UDP socket bound to port, supports broadcast and unconnected send/receive.
/// @note IPV4 only.
class UDPSocket {
  public:
	/// @brief Creates a UDP socket and binds it to the given @p port on all interfaces.
	/// @param port Local port to bind to.
	/// @return The bound socket or Error.
	static auto bind(std::uint16_t port) -> Result<UDPSocket>;

	/// @brief Enables or disables broadcast addresses
	/// @param enabled Whether broadcasting should be permitted.
	auto set_broadcast(bool enabled) const -> Result<void>; // NOLINT

	/// @brief Sends a single datagram to the given address.
	/// @param data Payload to send.
	/// @param address Destination host and port.
	[[nodiscard]] auto send_to(std::span<std::byte const> data, Address const& address) const -> Result<void>;
	/// @brief Receives a single datagram from any sender.
	/// @param buffer Destination buffer, must be at least the size of the incomming message.
	/// @param address Set to the sender's address on success.
	/// @return The number of bytes written into @p buffer
	[[nodiscard]] auto receive_from(std::span<std::byte> buffer, Address& address) const -> Result<std::size_t>;

	/// @brief Sets send and receive timeout for this connection only.
	/// @param timeout Maximum time to wait per send/receive call.
	auto set_timeout(std::chrono::milliseconds timeout) -> Result<void>;

  private:
	explicit UDPSocket(platform::SocketHandle fd) : m_socket(fd) {}

	Socket m_socket;
};
} // namespace bnet