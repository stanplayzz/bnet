#include "bnet/connection.hpp"
#include "bnet/context.hpp"
#include "bnet/listener.hpp"
#include "bnet/socket.hpp"
#include "platform.hpp"
#include <cstring>
#include <memory>
#include <utility>

#if defined(_WIN32)
#include <stdexcept>
#endif

namespace bnet {
namespace {
struct AddrInfoDeleter {
	void operator()(addrinfo* ptr) const noexcept { ::freeaddrinfo(ptr); }
};

struct Header {
	std::uint32_t version{};
	std::uint32_t size{};
};

constexpr std::uint32_t protocol_version_v = 1;

auto get_addr_info(char const* host, char const* port) -> std::unique_ptr<addrinfo, AddrInfoDeleter> {
	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (host == nullptr) { hints.ai_flags = AI_PASSIVE; }

	addrinfo* ptr{};
	auto const res = ::getaddrinfo(host, port, &hints, &ptr);
	if (res != 0) { return {}; }
	return std::unique_ptr<addrinfo, AddrInfoDeleter>{ptr};
}
} // namespace

auto Socket::send(std::span<std::byte const> data) const -> Result<void> {
	while (!data.empty()) {
		auto const res = platform::send(m_fd, data);

		if (res < 0) {
			if (platform::interrupted()) { continue; }
			if (platform::timed_out()) { return std::unexpected{Error::TimedOut}; }
			return std::unexpected{Error::SendFailed};
		}
		if (res == 0) { return std::unexpected{Error::ConnectionClosed}; }

		data = data.subspan(static_cast<std::size_t>(res));
	}

	return {};
}

auto Socket::receive(std::span<std::byte> buffer) const -> Result<std::size_t> {
	auto const res = platform::receive(m_fd, buffer);
	if (res < 0) {
		if (platform::timed_out()) { return std::unexpected{Error::TimedOut}; }
		return std::unexpected{Error::ReceiveFailed};
	}
	if (res == 0) { return std::unexpected{Error::ConnectionClosed}; }
	return res;
}

auto Socket::receive_exact(std::span<std::byte> buffer) const -> Result<void> {
	while (!buffer.empty()) {
		auto const res = platform::receive(m_fd, buffer);
		if (res < 0) {
			if (platform::interrupted()) { continue; }
			if (platform::timed_out()) { return std::unexpected{Error::TimedOut}; }
			return std::unexpected{Error::ReceiveFailed};
		}
		if (res == 0) { return std::unexpected{Error::ConnectionClosed}; }
		buffer = buffer.subspan(std::size_t(res));
	}

	return {};
}

Socket::Socket(Socket&& rhs) noexcept : m_fd(std::exchange(rhs.m_fd, platform::invalid_v)) {}

auto Socket::operator=(Socket&& rhs) noexcept -> Socket& {
	if (this != &rhs) {
		if (m_fd != platform::invalid_v) {
			platform::shutdown(m_fd);
			platform::close(m_fd);
		}
		m_fd = rhs.m_fd;
		rhs.m_fd = platform::invalid_v;
	}
	return *this;
}

Socket::~Socket() noexcept {
	if (m_fd != platform::invalid_v) {
		platform::shutdown(m_fd);
		platform::close(m_fd);
		m_fd = platform::invalid_v;
	}
}

auto Connection::connect(Address const& address) -> Result<Connection> {
	auto port = std::to_string(address.port);
	auto info = get_addr_info(address.host.c_str(), port.c_str());
	if (!info) { return std::unexpected{Error::AddressResolutionFailed}; }

	for (auto* ptr = info.get(); ptr != nullptr; ptr = ptr->ai_next) {
		auto socket_fd = platform::SocketHandle{};
		if ((socket_fd = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == platform::invalid_v) {
			continue;
		}
		if (::connect(socket_fd, ptr->ai_addr, platform::SockLen(ptr->ai_addrlen)) != platform::error_v) {
			return Connection{Socket{socket_fd}};
		}

		platform::close(socket_fd);
	}

	return std::unexpected{Error::ConnectFailed};
}

auto Connection::send(std::span<std::byte const> data) -> Result<void> {
	if (data.empty()) { return std::unexpected{Error::InvalidArgument}; }

	return m_socket.send(data);
}
auto Connection::receive(std::span<std::byte> buffer) -> Result<std::size_t> {
	if (buffer.empty()) { return std::unexpected{Error::InvalidArgument}; }

	return m_socket.receive(buffer);
}

auto Connection::receive_exact(std::span<std::byte> buffer) -> Result<void> {
	if (buffer.empty()) { return std::unexpected{Error::InvalidArgument}; }

	return m_socket.receive_exact(buffer);
}

auto Connection::send_framed(std::span<std::byte const> data) -> Result<void> {
	if (data.empty()) { return std::unexpected{Error::InvalidArgument}; }

	auto const header = Header{
		.version = htonl(protocol_version_v),
		.size = htonl(static_cast<std::uint32_t>(data.size())),
	};
	auto const header_bytes = std::as_bytes(std::span{&header, 1});

	if (auto result = m_socket.send(header_bytes); !result) { return result; }
	return m_socket.send(data);
}

auto Connection::receive_framed(std::span<std::byte> buffer) -> Result<std::size_t> {
	auto header = Header{};
	auto header_buf = std::as_writable_bytes(std::span{&header, 1});
	if (auto result = m_socket.receive_exact(header_buf); !result) { return std::unexpected{result.error()}; }

	auto const version = ntohl(header.version);
	auto const len = ntohl(header.size);

	if (version != protocol_version_v) { return std::unexpected{Error::ProtocolMismatch}; }
	if (len > buffer.size()) { return std::unexpected{Error::InvalidArgument}; }
	if (auto result = m_socket.receive_exact(buffer.first(len)); !result) { return std::unexpected{result.error()}; }
	return len;
}

auto Connection::set_no_delay(bool enabled) -> Result<void> {
	if (platform::set_no_delay(m_socket.fd(), enabled) == platform::error_v) {
		return std::unexpected{Error::SetSockOptFailed};
	}

	return {};
}

auto Connection::set_timeout(std::chrono::milliseconds timeout) -> Result<void> {
	if (platform::set_recv_timeout(m_socket.fd(), timeout.count()) == platform::error_v) {
		return std::unexpected{Error::SetSockOptFailed};
	}
	if (platform::set_send_timeout(m_socket.fd(), timeout.count()) == platform::error_v) {
		return std::unexpected{Error::SetSockOptFailed};
	}
	return {};
}

auto Listener::create(std::uint16_t port, int backlog) -> Result<Listener> {
	auto port_string = std::to_string(port);
	auto info = get_addr_info(nullptr, port_string.c_str());
	if (!info) { return std::unexpected{Error::AddressResolutionFailed}; }

	for (auto* ptr = info.get(); ptr != nullptr; ptr = ptr->ai_next) {
		auto socket_fd = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (socket_fd == platform::invalid_v) { continue; }

		if (ptr->ai_family == AF_INET6) { platform::set_v6_only(socket_fd, false); }
		platform::set_reuse_addr(socket_fd);

		if (::bind(socket_fd, ptr->ai_addr, platform::SockLen(ptr->ai_addrlen)) == platform::error_v) {
			platform::close(socket_fd);
			continue;
		}

		if (::listen(socket_fd, backlog) == platform::error_v) {
			platform::close(socket_fd);
			continue;
		}

		return Listener{Socket{socket_fd}};
	}

	return std::unexpected{Error::ListenFailed};
}

auto Listener::accept() -> Result<Connection> {
	for (;;) {
		auto fd = ::accept(m_socket.fd(), nullptr, nullptr);
		if (fd == platform::invalid_v) {
			if (platform::interrupted()) { continue; }
			return std::unexpected{Error::AcceptFailed};
		}

		return Connection{Socket{fd}};
	}
}

auto Listener::set_timeout(std::chrono::milliseconds timeout) -> Result<void> {
	if (platform::set_recv_timeout(m_socket.fd(), timeout.count()) == platform::error_v) {
		return std::unexpected{Error::SetSockOptFailed};
	}
	if (platform::set_send_timeout(m_socket.fd(), timeout.count()) == platform::error_v) {
		return std::unexpected{Error::SetSockOptFailed};
	}
	return {};
}

Context::Context() {
#if defined(_WIN32)
	WSADATA data{};

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) { throw std::runtime_error{"Failed to initialize Winsock"}; }

	if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2) {
		WSACleanup();
		throw std::runtime_error{"Winsock 2.2 unavailable"};
	}

	m_initialized = true;
#endif
}

Context::~Context() {
#if defined(_WIN32)
	if (m_initialized) { WSACleanup(); }
#endif
}
} // namespace bnet