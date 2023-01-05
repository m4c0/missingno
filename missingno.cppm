export module missingno;

namespace mno {
export template <typename T> class req {
  T m_val;
  const char *m_msg;

  constexpr req(T val, const char *msg) noexcept : m_val{val}, m_msg{msg} {}

  template <typename> friend class req;

public:
  using type = T;

  constexpr req() noexcept = default;
  constexpr req(T val) noexcept : m_val{val}, m_msg{} {}

  [[nodiscard]] constexpr req<T> otherwise(const char *m) const noexcept {
    return {m_val, m_msg == nullptr ? nullptr : m};
  };
  [[nodiscard]] constexpr T unwrap(T def) const noexcept {
    return m_msg == nullptr ? m_val : def;
  }

  [[nodiscard]] constexpr auto then(auto fn) const noexcept
      -> req<decltype(fn(T{}))> {
    if (m_msg == nullptr)
      return {fn(m_val), nullptr};
    return {{}, m_msg};
  }
  template <typename S, typename TT = T>
  [[nodiscard]] constexpr req<S> then(S TT::*m) const noexcept {
    return {(m_msg == nullptr) ? m_val.*m : S{}, m_msg};
  }

  [[nodiscard]] constexpr auto compose(auto fn) const noexcept
      -> req<typename decltype(fn(T{}))::type> {
    if (m_msg == nullptr)
      return fn(m_val);
    return {{}, m_msg};
  }

  [[nodiscard]] constexpr static req<T> failed(const char *m) noexcept {
    return {T{}, m};
  }
};
export template <typename T> req(T) -> req<T>;

export template <> class req<void> {
  const char *m_msg;

  constexpr req(const char *msg) noexcept : m_msg{msg} {}

  template <typename> friend class req;

public:
  constexpr req() noexcept = default;

  [[nodiscard]] constexpr req<void> otherwise(const char *m) const noexcept {
    return {m_msg == nullptr ? nullptr : m};
  };

  [[nodiscard]] constexpr auto then(auto fn) const noexcept
      -> req<decltype(fn())> {
    if (m_msg == nullptr)
      return {fn(), nullptr};
    return {{}, m_msg};
  }

  [[nodiscard]] constexpr auto compose(auto fn) const noexcept
      -> req<typename decltype(fn())::type> {
    if (m_msg == nullptr)
      return fn();
    return {{}, m_msg};
  }

  [[nodiscard]] constexpr static req<void> failed(const char *m) noexcept {
    return {m};
  }
};

static_assert([] {
  constexpr const auto flip = [](bool b) { return !b; };
  return req<bool>::failed("Some error message")
      .then(flip)
      .then(flip)
      .unwrap(true);
}());
static_assert([] { return req{true}.unwrap(false); }());

static_assert([] {
  return req{false}.compose([](bool) { return req{true}; }).unwrap(false);
}());

static_assert([] {
  return req<void>{}.then([] { return true; }).unwrap(false);
}());
static_assert([] {
  return req<void>{}.compose([] { return req{true}; }).unwrap(false);
}());

} // namespace mno
