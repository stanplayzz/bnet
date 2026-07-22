#pragma once
#include <cstdint>
#include <expected>
#include <string_view>

namespace bnet {
enum class Error : std::uint8_t {
	InvalidArgument,
	AddressResolutionFailed,
	SocketCreationFailed,
	BindFailed,
	ListenFailed,
	ConnectFailed,
	AcceptFailed,
	SendFailed,
	ReceiveFailed,
	ConnectionClosed,
	TimedOut,
};

constexpr auto to_string_view(Error const error) -> std::string_view {
	switch (error) {
	case Error::InvalidArgument: return "InvalidArgument";
	case Error::AddressResolutionFailed: return "AddressResolutionFailed";
	case Error::SocketCreationFailed: return "SocketCreationFailed";
	case Error::BindFailed: return "BindFailed";
	case Error::ListenFailed: return "ListenFailed";
	case Error::ConnectFailed: return "ConnectFailed";
	case Error::AcceptFailed: return "AcceptFailed";
	case Error::SendFailed: return "SendFailed";
	case Error::ReceiveFailed: return "ReceiveFailed";
	case Error::ConnectionClosed: return "ConnectionClosed";
	case Error::TimedOut: return "TimedOut";
	}
}

template <typename Type>
using Result = std::expected<Type, Error>;
} // namespace bnet