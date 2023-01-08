export module missingno:operators;
import :req;

namespace mno {
export template <typename A, typename B>
[[nodiscard]] constexpr auto operator+(const req<A> &a,
                                       const req<B> &b) noexcept {
  return combine(a, b, [](const auto &va, const auto &vb) { return va + vb; });
}
static_assert(req{3} + req{6} == req{9});
static_assert(req<int>::failed("failed") + req{6} ==
              req<int>::failed("failed"));
static_assert(req{3} + req<int>::failed("failed") ==
              req<int>::failed("failed"));
static_assert(req<int>::failed("this failed") +
                  req<int>::failed("also failed") ==
              req<int>::failed("this failed"));
} // namespace mno
