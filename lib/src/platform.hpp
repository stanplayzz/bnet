#pragma once
#include <cstdint>
#include <span>

#if defined(__linux__)

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

using SocketHandle = int;
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

inline auto receive(SocketHandle const socket, std::span<std::byte> buffer) -> std::int64_t {
	return ::recv(socket, buffer.data(), buffer.size(), 0);
}

inline auto set_reuse_addr(SocketHandle const socket) -> std::int64_t {
	static constexpr int value_v{1};
	return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &value_v, sizeof(value_v));
}

inline auto set_no_delay(SocketHandle const socket, bool enabled) -> std::int64_t {
	int value_v{enabled ? 1 : 0};
	return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &value_v, sizeof(value_v));
}

inline auto set_v6_only(SocketHandle socket, bool enabled) -> std::int64_t {
	int value_v{enabled ? 1 : 0};
	return ::setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, &value_v, sizeof(value_v));
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

#elif defined(_WIN32)

using SocketHandle = SOCKET;
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

inline auto receive(SocketHandle const socket, std::span<std::byte> buffer) -> std::int64_t {
	void* erased = buffer.data();
	return ::recv(socket, static_cast<char*>(erased), int(buffer.size()), 0);
}

inline auto set_reuse_addr(SocketHandle const socket) -> std::int64_t {
	static constexpr BOOL value_v{TRUE};
	void const* erased = &value_v;
	return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, static_cast<char const*>(erased), int(sizeof(value_v)));
}

inline auto set_no_delay(SocketHandle const socket, bool enabled) -> std::int64_t {
	BOOL value_v{enabled ? 1 : 0};
	void const* erased = &value_v;
	return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, static_cast<char const*>(erased), sizeof(value_v));
}

inline auto set_v6_only(SocketHandle const socket, bool enabled) -> std::int64_t {
	BOOL value_v{enabled ? 1 : 0};
	void const* erased = &value_v;
	return ::setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, static_cast<char const*>(erased), sizeof(value_v));
}

inline auto set_recv_timeout(SocketHandle const socket, std::int64_t millis) -> std::int64_t {
	DWORD value_v = static_cast<DWORD>(millis);
	void const* erased = &value_v;
	return ::setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, static_cast<char const*>(erased), sizeof(value_v));
}

inline auto set_send_timeout(SocketHandle const socket, std::int64_t millis) -> std::int64_t {
	DWORD value_v = static_cast<DWORD>(millis);
	void const* erased = &value_v;
	return ::setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, static_cast<char const*>(erased), sizeof(value_v));
}

#endif
} // namespace bnet::platform