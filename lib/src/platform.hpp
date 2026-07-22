#pragma once
#include <cstdint>
#include <span>

#if defined(__linux__)

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#elif defined(_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
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

inline auto close(SocketHandle const s) { ::close(s); }

inline auto send(SocketHandle const socket, std::span<std::byte const> data) -> std::int64_t {
	return ::send(socket, data.data(), data.size(), 0);
}

inline auto receive(SocketHandle const socket, std::span<std::byte> buffer) -> std::int64_t {
	return ::recv(socket, buffer.data(), buffer.size(), 0);
}

inline void set_reuse_addr(SocketHandle const socket) {
	static constexpr int value_v{1};
	::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &value_v, sizeof(value_v));
}

#elif defined(_WIN32)

using SocketHandle = SOCKET;
using SockLen = int;
constexpr auto invalid_v = INVALID_SOCKET;
constexpr auto error_v = SOCKET_ERROR;

inline void close(SocketHandle const s) { ::closesocket(s); }

inline auto send(SocketHandle const socket, std::span<std::byte const> data) -> std::int64_t {
	void const* erased = data.data();
	return ::send(socket, static_cast<char const*>(erased), int(data.size()), 0);
}

inline auto receive(SocketHandle const socket, std::span<std::byte> buffer) -> std::int64_t {
	void* erased = buffer.data();
	return ::recv(socket, static_cast<char*>(erased), int(buffer.size()), 0);
}

inline void set_reuse_addr(SocketHandle const socket) {
	static constexpr BOOL value_v{TRUE};
	void const* erased = &value_v;
	::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, static_cast<char const*>(erased), int(sizeof(value_v)));
}

#endif
} // namespace bnet::platform