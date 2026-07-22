#include "bnet/connection.hpp"
#include "bnet/context.hpp"
#include "bnet/listener.hpp"
#include "bnet/socket.hpp"
#include "platform.hpp"
#include <cassert>
#include <memory>

namespace bnet {
namespace {
struct AddrInfoDeleter {
	void operator()(addrinfo* ptr) const noexcept { ::freeaddrinfo(ptr); }
};

auto get_addr_info(char const* host, char const* port) -> std::unique_ptr<addrinfo, AddrInfoDeleter> {
	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* ptr{};
	auto const res = ::getaddrinfo(host, port, &hints, &ptr);
	if (res != 0) { return {}; }
	return std::unique_ptr<addrinfo, AddrInfoDeleter>{ptr};
}
} // namespace

auto Socket::send(std::span<std::byte const> data) const -> Result<void> {
	while (!data.empty()) {
		auto const res = platform::send(m_fd, data);

		if (res < 0) { return std::unexpected{Error::SendFailed}; }
		if (res == 0) { return std::unexpected{Error::ConnectionClosed}; }

		data = data.subspan(static_cast<std::size_t>(res));
	}

	return {};
}

auto Socket::receive(std::span<std::byte> buffer) const -> Result<std::size_t> {
	auto const res = platform::receive(m_fd, buffer);
	if (res < 0) { return std::unexpected{Error::ReceiveFailed}; }
	if (res == 0) { return std::unexpected{Error::ConnectionClosed}; }
	return res;
}

auto Socket::receive_exact(std::span<std::byte> buffer) const -> Result<void> {
	while (!buffer.empty()) {
		auto const res = platform::receive(m_fd, buffer);
		if (res < 0) { return std::unexpected{Error::ReceiveFailed}; }
		if (res == 0) { return std::unexpected{Error::ConnectionClosed}; }
		buffer = buffer.subspan(std::size_t(res));
	}

	return {};
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
		if (::connect(socket_fd, ptr->ai_addr, ptr->ai_addrlen) != platform::error_v) {
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

auto Listener::create(std::uint16_t port, int backlog) -> Result<Listener> {
	auto port_string = std::to_string(port);
	auto info = get_addr_info(nullptr, port_string.c_str());
	if (!info) { return std::unexpected{Error::AddressResolutionFailed}; }

	for (auto* ptr = info.get(); ptr != nullptr; ptr = ptr->ai_next) {
		auto socket_fd = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (socket_fd == platform::invalid_v) { continue; }

		platform::set_reuse_addr(socket_fd);

		if (::bind(socket_fd, ptr->ai_addr, ptr->ai_addrlen) == platform::error_v) {
			platform::close(socket_fd);
			continue;
		}

		if (::listen(socket_fd, backlog) == platform::error_v) {
			platform::close(socket_fd);
			continue;
		}

		return Listener{socket_fd};
	}

	return std::unexpected{Error::ListenFailed};
}

auto Listener::accept() -> Result<Connection> {
	auto fd = ::accept(m_socket.fd(), nullptr, nullptr);
	if (fd == platform::invalid_v) { return std::unexpected{Error::AcceptFailed}; }

	return Connection{Socket{fd}};
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