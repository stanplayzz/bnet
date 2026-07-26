#pragma once

namespace bnet {
/// @brief RAII wrapper for platform network stack initialization.
///
/// A single context must outlive every Socket/Connection/Listener in the program.
class Context {
  public:
	Context();
	~Context();

	Context(Context const&) = delete;
	Context& operator=(Context const&) = delete;
	Context(Context&&) noexcept = delete;
	auto operator=(Context&&) noexcept = delete;

  private:
#if defined(_WIN32)
	bool m_initialized{};
#endif
};
} // namespace bnet