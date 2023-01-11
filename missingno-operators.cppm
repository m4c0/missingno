export module missingno:operators;
import :req;

namespace mno {
export template <typename A, typename B>
[[nodiscard]] constexpr auto operator+(const req<A> &a, B b) noexcept {
  return a.map([&](auto a) { return a + b; });
}
static_assert((req{3} + 2) == req{5});
static_assert((req<int>::failed("failed") + 5) == req<int>::failed("failed"));

export template <typename A, typename B>
[[nodiscard]] constexpr auto operator-(const req<A> &a, B b) noexcept {
  return a.map([&](auto a) { return a - b; });
}
static_assert((req{3} - 2) == req{1});
static_assert((req<int>::failed("failed") - 5) == req<int>::failed("failed"));

export template <typename A, typename B>
[[nodiscard]] constexpr auto operator&(const req<A> &a, B b) noexcept {
  return a.map([&](auto a) { return a & b; });
}
static_assert((req{0b1111} & 0b0100) == req{0b0100});
static_assert((req<int>::failed("failed") & 1) == req<int>::failed("failed"));

export template <typename A, typename B>
[[nodiscard]] constexpr auto operator|(const req<A> &a, B b) noexcept {
  return a.map([&](auto a) { return a | b; });
}
static_assert((req{0b1010} | 0b0101) == req{0b1111});
static_assert((req<int>::failed("failed") | 1) == req<int>::failed("failed"));

export template <typename A, typename B>
[[nodiscard]] constexpr auto operator<<(const req<A> &a, B b) noexcept {
  return a.map([&](auto a) { return a << b; });
}
static_assert((req{0xF} << 4) == req{0xF0});
static_assert((req<int>::failed("failed") << 8) == req<int>::failed("failed"));

export template <typename A, typename B>
[[nodiscard]] constexpr auto operator>>(const req<A> &a, B b) noexcept {
  return a.map([&](auto a) { return a >> b; });
}
static_assert((req{0xF0} >> 4) == req{0xF});
static_assert((req<int>::failed("failed") >> 4) == req<int>::failed("failed"));

export template <typename A, typename B>
[[nodiscard]] constexpr auto operator+(const req<A> &a,
                                       const req<B> &b) noexcept {
  return combine([](const auto &va, const auto &vb) { return va + vb; }, a, b);
}
static_assert(req{3} + req{6} == req{9});

export template <typename A, typename B>
[[nodiscard]] constexpr auto operator-(const req<A> &a,
                                       const req<B> &b) noexcept {
  return combine([](const auto &va, const auto &vb) { return va - vb; }, a, b);
}
static_assert(req{3} - req{6} == req{-3});

export template <typename A, typename B>
[[nodiscard]] constexpr auto operator&(const req<A> &a,
                                       const req<B> &b) noexcept {
  return combine([](const auto &va, const auto &vb) { return va & vb; }, a, b);
}
static_assert((req{0b101} & req{0b100}) == req{0b100});

export template <typename A, typename B>
[[nodiscard]] constexpr auto operator|(const req<A> &a,
                                       const req<B> &b) noexcept {
  return combine([](const auto &va, const auto &vb) { return va | vb; }, a, b);
}
static_assert((req{0b100} | req{0b001}) == req{0b101});

static_assert(((req{0x5} << 4) | req{0xA}) == req{0x5A});
} // namespace mno
