export module missingno:opt;
import :value;
import traits;

namespace mno {
export template <typename T> class opt {
  value<T> m_val{};
  bool m_filled{};

public:
  using type = T;

  constexpr opt() noexcept = default;
  template <typename TT>
  constexpr explicit opt(TT &&val) noexcept
      : m_val{traits::fwd<TT>(val)}, m_filled{true} {}

  constexpr opt(const opt<T> &) noexcept = default;
  constexpr opt(opt<T> &&) noexcept = default;
  constexpr opt &operator=(const opt<T> &) noexcept = default;
  constexpr opt &operator=(opt<T> &&) noexcept = default;

  template <typename TT> constexpr opt &operator=(TT &&v) noexcept {
    m_val = {traits::fwd<TT>(v)};
    m_filled = true;
    return *this;
  }

  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return m_filled;
  }
  [[nodiscard]] constexpr bool operator==(const opt<T> &o) const noexcept {
    if (m_filled != o.m_filled)
      return false;
    if (!m_filled)
      return true;

    return m_val == o.m_val;
  }
  template <typename TT>
  [[nodiscard]] constexpr bool operator==(TT v) const noexcept {
    return m_filled && m_val.v == v;
  }

  template <typename TT>
    requires is_assignable_from<T, TT>
  [[nodiscard]] constexpr T unwrap(TT def) const noexcept {
    return m_filled ? m_val.v : def;
  }
  template <typename TT>
    requires is_not_assignable_from<T, TT> && is_same_v<T, call_result_t<TT>>
  [[nodiscard]] constexpr T unwrap(TT def) const noexcept {
    return m_filled ? m_val.v : def();
  }

  [[nodiscard]] constexpr auto map(auto fn) noexcept {
    using O = typename decltype(mno::map(m_val, fn))::type;
    return m_filled ? opt<O>{mno::map(m_val, fn)} : opt<O>{};
  }
  [[nodiscard]] constexpr auto map(auto fn) const noexcept {
    using O = typename decltype(mno::map(m_val, fn))::type;
    return m_filled ? opt<O>{mno::map(m_val, fn)} : opt<O>{};
  }

  [[nodiscard]] constexpr auto fmap(auto fn) noexcept {
    using RO = typename decltype(mno::map(m_val, fn))::type;
    using O = typename RO::type;
    return m_filled ? mno::map(m_val, fn).v : opt<O>{};
  }
  [[nodiscard]] constexpr auto fmap(auto fn) const noexcept {
    using RO = typename decltype(mno::map(m_val, fn))::type;
    using O = typename RO::type;
    return m_filled ? mno::map(m_val, fn).v : opt<O>{};
  }

  constexpr void consume(auto fn) noexcept {
    if (m_filled)
      fn(m_val.v);
  }
};
template <typename T> opt(T) -> opt<T>;

static_assert(opt{3});
static_assert(!opt<int>{});

static_assert(opt{3} == opt{3});
static_assert(opt{2} != opt{3});
static_assert(opt{3} != opt<int>{});
static_assert(opt<int>{} != opt{3});
static_assert(opt<int>{} == opt<int>{});

static_assert([] {
  constexpr const auto flip = [](bool b) { return !b; };
  return opt<bool>{}.map(flip).map(flip).unwrap(true);
}());
static_assert(opt{true}.unwrap(false));
static_assert(opt{true}.unwrap([] { return false; }));
static_assert(opt<bool>{}.unwrap([] { return true; }));

static_assert(opt{false}.fmap([](bool b) { return opt{!b}; }).unwrap(false));

static_assert(opt<int>{0}.map([](int) { return true; }).unwrap(false));

static_assert(opt{3} == 3);
static_assert(opt{2} != 3);
static_assert(opt<int>{} != 3);

static_assert((opt{3} = 2) == 2);
} // namespace mno
