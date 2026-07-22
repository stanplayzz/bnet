#pragma once

namespace bnet {
// Context class needed for Windows WSA initialization, not needed on other platforms.
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