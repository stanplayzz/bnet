#pragma once
#include "bnet/error.hpp"
#include "bnet/socket_handle.hpp"
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

	[[nodiscard]] auto send(std::span<std::byte const> data) const -> Result<void>;
	[[nodiscard]] auto receive(std::span<std::byte> buffer) const -> Result<std::size_t>;
	[[nodiscard]] auto receive_exact(std::span<std::byte> buffer) const -> Result<void>;

	[[nodiscard]] auto fd() const noexcept -> platform::SocketHandle { return m_fd; }

  private:
	platform::SocketHandle m_fd{static_cast<platform::SocketHandle>(-1)};
};
} // namespace bnet