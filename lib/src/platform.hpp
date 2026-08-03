#pragma once
#include "bnet/socket_handle.hpp"
#include <cstdint>
#include <span>

#if defined(__linux__)

#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>

#elif defined(_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <Winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#else
#error Unsupported platform
#endif

namespace bnet::platform {
#if defined(__linux__)

using SockLen = socklen_t;
constexpr auto invalid_v = SocketHandle{-1};
constexpr auto error_v = SocketHandle{-1};

inline auto close(SocketHandle const socket) -> std::int64_t { return ::close(socket); }

inline auto shutdown(SocketHandle const socket) -> std::int64_t { return ::shutdown(socket, SHUT_RDWR); }

inline auto interrupted() -> bool { return errno == EINTR; }

inline auto timed_out() -> bool { return errno == EAGAIN || errno == EWOULDBLOCK; }

inline auto send(SocketHandle const socket, std::span<std::byte const> data) -> std::int64_t {
	return ::send(socket, data.data(), data.size(), MSG_NOSIGNAL);
}

inline auto sendto(SocketHandle const socket, std::span<std::byte const> data, sockaddr* addr, SockLen addr_len)
	-> std::int64_t {
	return ::sendto(socket, data.data(), data.size(), 0, addr, addr_len);
}

inline auto receive(SocketHandle const socket, std::span<std::byte> buffer) -> std::int64_t {
	return ::recv(socket, buffer.data(), buffer.size(), 0);
}

inline auto recvfrom(SocketHandle const socket, std::span<std::byte> buffer, sockaddr* addr, SockLen* addr_len)
	-> std::int64_t {
	return ::recvfrom(socket, buffer.data(), buffer.size(), 0, addr, addr_len);
}

inline auto set_reuse_addr(SocketHandle const socket) -> std::int64_t {
	static constexpr int value_v{1};
	return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &value_v, sizeof(value_v));
}

inline auto set_no_delay(SocketHandle const socket, bool enabled) -> std::int64_t {
	int value{enabled ? 1 : 0};
	return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
}

inline auto set_v6_only(SocketHandle socket, bool enabled) -> std::int64_t {
	int value{enabled ? 1 : 0};
	return ::setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, &value, sizeof(value));
}

inline auto set_recv_timeout(SocketHandle const socket, std::int64_t millis) -> std::int64_t {
	auto tv = timeval{};
	tv.tv_sec = static_cast<time_t>(millis / 1000);
	tv.tv_usec = static_cast<suseconds_t>((millis % 1000) * 1000);
	return ::setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

inline auto set_send_timeout(SocketHandle const socket, std::int64_t millis) -> std::int64_t {
	auto tv = timeval{};
	tv.tv_sec = static_cast<time_t>(millis / 1000);
	tv.tv_usec = static_cast<suseconds_t>((millis % 1000) * 1000);
	return ::setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

inline auto set_broadcast(SocketHandle const socket, bool enabled) -> std::int64_t {
	int value{enabled ? 1 : 0};
	return ::setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &value, sizeof(value));
}

inline auto set_non_blocking(SocketHandle fd, bool enabled) -> std::int64_t {
	auto flags = ::fcntl(fd, F_GETFL, 0);
	if (flags == -1) { return -1; }
	flags = enabled ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
	return ::fcntl(fd, F_SETFL, flags); // NOLINT
}

inline auto would_block() -> bool { return errno == EWOULDBLOCK || errno == EAGAIN; }

#elif defined(_WIN32)

using SockLen = int;
constexpr auto invalid_v = INVALID_SOCKET;
constexpr auto error_v = SOCKET_ERROR;

inline auto close(SocketHandle const socket) -> std::int64_t { return ::closesocket(socket); }

inline auto shutdown(SocketHandle const socket) -> std::int64_t { return ::shutdown(socket, SD_BOTH); }

inline auto interrupted() -> bool { return false; }

inline auto timed_out() -> bool { return WSAGetLastError() == WSAETIMEDOUT; }

inline auto send(SocketHandle const socket, std::span<std::byte const> data) -> std::int64_t {
	void const* erased = data.data();
	return ::send(socket, static_cast<char const*>(erased), int(data.size()), 0);
}

inline auto sendto(SocketHandle const socket, std::span<std::byte const> data, sockaddr* addr, SockLen addr_len)
	-> std::int64_t {
	void const* erased = data.data();
	return ::sendto(socket, static_cast<char const*>(erased), static_cast<int>(data.size()), 0, addr, addr_len);
}

inline auto receive(SocketHandle const socket, std::span<std::byte> buffer) -> std::int64_t {
	void* erased = buffer.data();
	return ::recv(socket, static_cast<char*>(erased), int(buffer.size()), 0);
}

inline auto recvfrom(SocketHandle const socket, std::span<std::byte> buffer, sockaddr* addr, SockLen* addr_len)
	-> std::int64_t {
	void* erased = buffer.data();
	return ::recvfrom(socket, static_cast<char*>(erased), static_cast<int>(buffer.size()), 0, addr, addr_len);
}

inline auto set_reuse_addr(SocketHandle const socket) -> std::int64_t {
	static constexpr BOOL value_v{TRUE};
	void const* erased = &value_v;
	return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, static_cast<char const*>(erased), int(sizeof(value_v)));
}

inline auto set_no_delay(SocketHandle const socket, bool enabled) -> std::int64_t {
	BOOL value{enabled ? 1 : 0};
	void const* erased = &value;
	return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, static_cast<char const*>(erased), sizeof(value));
}

inline auto set_v6_only(SocketHandle const socket, bool enabled) -> std::int64_t {
	BOOL value{enabled ? 1 : 0};
	void const* erased = &value;
	return ::setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, static_cast<char const*>(erased), sizeof(value));
}

inline auto set_recv_timeout(SocketHandle const socket, std::int64_t millis) -> std::int64_t {
	DWORD value = static_cast<DWORD>(millis);
	void const* erased = &value;
	return ::setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, static_cast<char const*>(erased), sizeof(value));
}

inline auto set_send_timeout(SocketHandle const socket, std::int64_t millis) -> std::int64_t {
	DWORD value = static_cast<DWORD>(millis);
	void const* erased = &value;
	return ::setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, static_cast<char const*>(erased), sizeof(value));
}

inline auto set_broadcast(SocketHandle const socket, bool enabled) -> std::int64_t {
	BOOL value{enabled ? 1 : 0};
	void const* erased = &value;
	return ::setsockopt(socket, SOL_SOCKET, SO_BROADCAST, static_cast<char const*>(erased), sizeof(value));
}

inline auto set_non_blocking(SocketHandle fd, bool enabled) -> std::int64_t {
	u_long mode = enabled ? 1 : 0;
	return ::ioctlsocket(fd, FIONBIO, &mode);
}

inline auto would_block() -> bool { return WSAGetLastError() == WSAEWOULDBLOCK; }

#endif
} // namespace bnet::platform