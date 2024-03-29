export module missingno:req;
import :value;
import traits;

using namespace traits;

namespace mno {
class erred {};

export template <typename T> class req;

export template <typename T> class req {
  value<T> m_val{};
  const char *m_msg{};

  constexpr req(value<T> val, const char *msg) noexcept
      : m_val{val}, m_msg{msg} {}
  constexpr req(erred, const char *msg) noexcept : m_val{}, m_msg{msg} {}

  [[nodiscard]] constexpr bool is_valid() const noexcept {
    return m_msg == nullptr;
  }
  [[nodiscard]] constexpr auto val() const noexcept { return m_val.v; }
  [[nodiscard]] constexpr auto msg() const noexcept { return m_msg; }

  template <typename TT> friend class req;

public:
  using type = T;

  constexpr req() noexcept = default;
  template <typename TT>
  constexpr explicit req(TT &&val) noexcept : m_val{fwd<TT>(val)}, m_msg{} {}

  constexpr req(const req<T> &) noexcept = default;
  constexpr req(req<T> &&) noexcept = default;
  constexpr req &operator=(const req<T> &) noexcept = default;
  constexpr req &operator=(req<T> &&) noexcept = default;

  [[nodiscard]] constexpr static req<T> failed(const char *m) noexcept {
    return {erred{}, m};
  }
  [[nodiscard]] constexpr req<T> if_failed(const char *m) const noexcept {
    return {m_val, m_msg == nullptr ? nullptr : m};
  }
  [[nodiscard]] constexpr req<T> if_failed(auto fn) const noexcept {
    return m_msg == nullptr ? req<T>{m_val, nullptr} : fn(m_msg);
  }
  [[nodiscard]] constexpr auto assert(auto fn, const char *m) noexcept {
    if (m_msg != nullptr)
      return req<T>{erred{}, m_msg};
    if (!mno::map(m_val, fn).v)
      return req<T>{erred{}, m};
    return req<T>{traits::move(m_val.v)};
  }

  [[nodiscard]] constexpr auto take(auto errfn) {
    if (m_msg != nullptr) {
      errfn(m_msg);
    }
    return move_out(m_val);
  }

  template <typename TT>
    requires is_assignable_from<T, TT>
  [[nodiscard]] constexpr T unwrap(TT def) const noexcept {
    return m_msg == nullptr ? m_val.v : def;
  }
  template <typename TT>
    requires is_not_assignable_from<T, TT> && is_same_v<T, call_result_t<TT>>
  [[nodiscard]] constexpr T unwrap(TT def) const noexcept {
    return m_msg == nullptr ? m_val.v : def();
  }
  template <typename TT>
  [[nodiscard]] constexpr req<T> otherwise(TT def) const noexcept {
    return {value<T>{m_msg == nullptr ? m_val.v : def}, nullptr};
  }

  [[nodiscard]] constexpr auto map(auto fn) noexcept {
    using O = typename decltype(mno::map(m_val, fn))::type;

    if (m_msg == nullptr)
      return req<O>{mno::map(m_val, fn)};
    return req<O>{erred{}, m_msg};
  }
  [[nodiscard]] constexpr auto map(auto fn) const noexcept {
    using O = typename decltype(mno::map(m_val, fn))::type;

    if (m_msg == nullptr)
      return req<O>{mno::map(m_val, fn), nullptr};
    return req<O>{erred{}, m_msg};
  }

  [[nodiscard]] constexpr auto fmap(auto fn) noexcept {
    using RO = typename decltype(mno::map(m_val, fn))::type;
    using O = typename RO::type;

    if (m_msg == nullptr)
      return mno::map(m_val, fn).v;
    return req<O>{erred{}, m_msg};
  }
  [[nodiscard]] constexpr auto fmap(auto fn) const noexcept {
    using RO = typename decltype(mno::map(m_val, fn))::type;
    using O = typename RO::type;

    if (m_msg == nullptr)
      return mno::map(m_val, fn).v;
    return req<O>{erred{}, m_msg};
  }

  [[nodiscard]] constexpr req<T>
  otherwise(void_consumer auto fn) const noexcept {
    if (m_msg == nullptr)
      return req<T>{m_val, nullptr};

    return req<T>{mno::map(value<void>{}, fn), nullptr};
  }

  [[nodiscard]] constexpr bool operator==(const req<T> &o) const noexcept {
    if ((m_msg != nullptr) && (o.m_msg != nullptr))
      return true;
    if ((m_msg != nullptr) != (o.m_msg != nullptr))
      return false;

    return m_val == o.m_val;
  }
  template <typename TT>
  [[nodiscard]] constexpr bool operator==(TT v) const noexcept {
    return (m_msg == nullptr) && m_val.v == v;
  }
};
template <typename T> req(T) -> req<T>;

static_assert(req{3} == req{3});
static_assert(req{2} != req{3});
static_assert(req{3} != req<int>::failed(""));
static_assert(req<int>::failed("") != req{3});
// TODO: static_assert(req<int>::failed("") != req<int>::failed("a"));
static_assert(req<int>::failed("a") == req<int>::failed("a"));

static_assert(req{3}.if_failed("b") == req{3});
static_assert(req<int>::failed("a").if_failed("b") == req<int>::failed("b"));
static_assert(req{3}.if_failed([](auto msg) { return req{99}; }) == req{3});
static_assert(req<int>::failed("a").if_failed([](auto msg) {
  return req{99};
}) == req{99});

static_assert([] {
  constexpr const auto flip = [](bool b) { return !b; };
  return req<bool>::failed("Some error message")
      .map(flip)
      .map(flip)
      .unwrap(true);
}());
static_assert(req{true}.unwrap(false));

static_assert(req{true}.unwrap([] { return false; }));
static_assert(req<bool>::failed("Error").unwrap([] { return true; }));

static_assert(req<bool>::failed("Error").otherwise(true).unwrap(false));
static_assert(
    req<bool>::failed("Error").otherwise([] { return true; }).unwrap(false));

static_assert(req<int>::failed("fail").assert([](auto) { return false; }, "") ==
              req<int>::failed("fail"));
static_assert(req<int>::failed("fail").assert([](auto) { return true; }, "") ==
              req<int>::failed("fail"));
static_assert(req<int>{}.assert([](auto) { return false; }, "asserted") ==
              req<int>::failed("asserted"));
static_assert(req<int>{}.assert([](auto) { return true; }, "") == req<int>{});
static_assert(req{3}.assert([](auto v) { return v == 3; }, "") == req{3});

static_assert(req{false}.fmap([](bool b) { return req{!b}; }).unwrap(false));

static_assert(req<void>{}.map([] { return true; }).unwrap(false));

static_assert(req<void>{}
                  .map([] {})
                  .otherwise((void (*)()) nullptr) // should never happen
                  .fmap([] { return req<void>{}; })
                  .fmap([] { return req{true}; })
                  .unwrap(false));

static_assert(req{3} == 3);
static_assert(req{2} != 3);
static_assert(req<int>::failed("") != 3);

export template <typename A, typename... B>
[[nodiscard]] constexpr auto combine(auto fn, const req<A> &a,
                                     const req<B> &...b) noexcept {
  return a.fmap([&](auto a) {
    return combine([&](B... b) { return fn(a, b...); }, b...);
  });
}
export template <typename A>
[[nodiscard]] constexpr auto combine(auto fn, const req<A> &a) noexcept {
  return a.map(fn);
}
static_assert(combine([](auto a, auto b) { return a + b; }, req{3}, req{6}) ==
              req{9});
static_assert(combine([](auto, auto) {}, req<int>::failed("failed"), req{6}) ==
              req<void>::failed("failed"));
static_assert(combine([](auto, auto) {}, req{3}, req<int>::failed("failed")) ==
              req<void>::failed("failed"));
static_assert(combine([](auto, auto) {}, req<int>::failed("this failed"),
                      req<int>::failed("also failed")) ==
              req<void>::failed("this failed"));

static_assert([] {
  struct s {
    constexpr s() = default;
    constexpr s(s &&) = default;
    constexpr s &operator=(s &&) = default;
    constexpr s(const s &) = delete;
    constexpr s &operator=(const s &) = delete;
    constexpr bool non_const() { return true; }
  };
  return req<s>()
      .assert([](auto &&o) { return true; }, "failed")
      .fmap([](auto &&o) { return req{static_cast<s &&>(o)}; })
      .map([](auto &&o) { return o.non_const(); })
      .unwrap(false);
}());
}; // namespace mno
