#pragma once
#include "bnet/error.hpp"
#include "platform.hpp"
#include <span>
#include <utility>

namespace bnet {
class Socket {
  public:
	explicit Socket(platform::SocketHandle fd) : m_fd(fd) {}

	Socket(Socket const&) = delete;
	Socket& operator=(Socket const&) = delete;
	Socket(Socket&& rhs) noexcept : m_fd(std::exchange(rhs.m_fd, platform::invalid_v)) {}
	auto operator=(Socket&& rhs) noexcept -> Socket& {
		if (this != &rhs) {
			if (m_fd != platform::invalid_v) { platform::close(m_fd); }
			m_fd = rhs.m_fd;
			rhs.m_fd = platform::invalid_v;
		}
		return *this;
	}

	~Socket() noexcept {
		if (m_fd != platform::invalid_v) { platform::close(m_fd); }
	}

	[[nodiscard]] auto send(std::span<std::byte const> data) const -> Result<void>;
	[[nodiscard]] auto receive(std::span<std::byte> buffer) const -> Result<std::size_t>;
	[[nodiscard]] auto receive_exact(std::span<std::byte> buffer) const -> Result<void>;

	[[nodiscard]] auto fd() const noexcept -> platform::SocketHandle { return m_fd; }

  private:
	platform::SocketHandle m_fd{platform::invalid_v};
};
} // namespace bnet