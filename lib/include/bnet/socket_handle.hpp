#pragma once

#if defined(__linux__)

namespace bnet::platform {
using SocketHandle = int;
} // namespace bnet::platform

#elif defined(_WIN32)

#if defined(_WIN64)
using SocketHandleRaw = unsigned long long;
#else
using SocketHandleRaw = unsigned int;
#endif

namespace bnet::platform {
using SocketHandle = SocketHandleRaw;
} // namespace bnet::platform

#else
#error Unsupported platform
#endif