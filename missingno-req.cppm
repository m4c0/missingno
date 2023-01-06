export module missingno:req;
import :value;

namespace mno {
class erred {};

export template <typename T> class req;

export template <typename T> class req {
  value<T> m_val;
  const char *m_msg;

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
  template <typename TT> constexpr req(TT val) noexcept : m_val{val}, m_msg{} {}

  [[nodiscard]] constexpr static req<T> failed(const char *m) noexcept {
    return {erred{}, m};
  }
  [[nodiscard]] constexpr req<T> if_failed(const char *m) const noexcept {
    return {m_val, m_msg == nullptr ? nullptr : m};
  };

  template <typename TT>
  [[nodiscard]] constexpr T unwrap(TT def) const noexcept {
    return m_msg == nullptr ? m_val.v : def;
  }
  template <typename TT>
  [[nodiscard]] constexpr req<T> otherwise(TT def) const noexcept {
    return {value<T>{m_msg == nullptr ? m_val.v : def}, nullptr};
  }

  [[nodiscard]] constexpr auto map(auto fn) const noexcept {
    using O = typename decltype(mno::map(m_val, fn))::type;

    if (m_msg == nullptr)
      return req<O>{mno::map(m_val, fn), nullptr};
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
};
template <typename T> req(T) -> req<T>;

static_assert([] {
  constexpr const auto flip = [](bool b) { return !b; };
  return req<bool>::failed("Some error message")
      .map(flip)
      .map(flip)
      .unwrap(true);
}());
static_assert(req{true}.unwrap(false));

static_assert(req<bool>::failed("Error").otherwise(true).unwrap(false));
static_assert(
    req<bool>::failed("Error").otherwise([] { return true; }).unwrap(false));

static_assert(req{false}.fmap([](bool b) { return req{!b}; }).unwrap(false));

static_assert(req<void>{}.map([] { return true; }).unwrap(false));

static_assert(req<void>{}
                  .map([] {})
                  .otherwise([] { throw 0; })
                  .fmap([] { return req<void>{}; })
                  .fmap([] { return req{true}; })
                  .unwrap(false));
}; // namespace mno
