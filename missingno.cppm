export module missingno;

namespace mno {
export template <typename T> class req {
  T m_val;
  const char *m_msg;

  constexpr req(T val, const char *msg) noexcept : m_val{val}, m_msg{msg} {}

  template <typename> friend class req;

public:
  constexpr req() noexcept = default;
  constexpr req(T val) noexcept : m_val{val}, m_msg{} {}

  [[nodiscard]] constexpr req<T> otherwise(const char *m) const noexcept {
    return {m_val, m_msg == nullptr ? nullptr : m};
  };
  [[nodiscard]] constexpr T unwrap(T def) const noexcept {
    return m_msg == nullptr ? m_val : def;
  }

  [[nodiscard]] constexpr auto then(auto fn) const noexcept(noexcept(fn(T{})))
      -> req<decltype(fn(T{}))> {
    if (m_msg == nullptr)
      return {fn(m_val), nullptr};
    return {{}, m_msg};
  }
  template <typename S, typename TT = T>
  [[nodiscard]] constexpr req<S> then(S TT::*m) const noexcept {
    return {(m_msg == nullptr) ? m_val.*m : S{}, m_msg};
  }

  [[nodiscard]] constexpr static req<T> failed(const char *m) noexcept {
    return {T{}, m};
  }
};
template <typename T> req(T) -> req<T>;

static_assert([] {
  constexpr const auto flip = [](bool b) { return !b; };
  return req<bool>::failed("Some error message")
      .then(flip)
      .then(flip)
      .unwrap(true);
}());
static_assert([] { return req{true}.unwrap(false); }());
} // namespace mno
