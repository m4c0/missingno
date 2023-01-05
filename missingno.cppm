export module missingno;

namespace mno {
class erred {};

template <typename A, typename B> struct is_same {
  static constexpr const auto value = false;
};
template <typename A> struct is_same<A, A> {
  static constexpr const auto value = true;
};
static_assert(is_same<void, void>::value);
static_assert(!is_same<int, void>::value);

template <typename A, typename B>
concept same_as = is_same<A, B>::value;
template <typename T, typename V>
concept consumer = requires(T t, V v) {
                     { t(v) } -> same_as<void>;
                   };
template <typename T>
concept void_consumer = requires(T t) {
                          { t() } -> same_as<void>;
                        };

export template <typename T> class req;

export template <> class req<void> {
  const char *m_msg;

  constexpr req(const char *msg) noexcept : m_msg{msg} {}
  constexpr req(erred, const char *msg) noexcept : m_msg{msg} {}

  template <typename> friend class req;

public:
  using type = void;

  constexpr req() noexcept = default;

  [[nodiscard]] constexpr req<void> otherwise(const char *m) const noexcept {
    return {m_msg == nullptr ? nullptr : m};
  };

  template <typename Fn>
    requires(!void_consumer<Fn>)
  [[nodiscard]] constexpr auto then(Fn &&fn) const noexcept
      -> req<decltype(fn())> {
    if (m_msg == nullptr)
      return {fn(), nullptr};
    return {erred{}, m_msg};
  }

  template <typename Fn>
    requires(void_consumer<Fn>)
  [[nodiscard]] constexpr auto then(Fn &&fn) const noexcept -> req<void> {
    if (m_msg == nullptr) {
      fn();
      return {};
    }
    return {erred{}, m_msg};
  }

  [[nodiscard]] constexpr auto compose(auto fn) const noexcept
      -> req<typename decltype(fn())::type> {
    if (m_msg == nullptr)
      return fn();
    return {erred{}, m_msg};
  }

  [[nodiscard]] constexpr req<void> otherwise(auto fn) const
      noexcept(noexcept(fn())) {
    if (m_msg != nullptr)
      fn();
    return {};
  }

  [[nodiscard]] constexpr static req<void> failed(const char *m) noexcept {
    return {m};
  }
};

export template <typename T> class req {
  T m_val;
  const char *m_msg;

  constexpr req(T val, const char *msg) noexcept : m_val{val}, m_msg{msg} {}
  constexpr req(erred, const char *msg) noexcept : m_val{}, m_msg{msg} {}

  template <typename> friend class req;

public:
  using type = T;

  constexpr req() noexcept = default;
  constexpr req(T val) noexcept : m_val{val}, m_msg{} {}

  [[nodiscard]] constexpr req<T> if_failed(const char *m) const noexcept {
    return {m_val, m_msg == nullptr ? nullptr : m};
  };

  [[nodiscard]] constexpr T unwrap(T def) const noexcept {
    return m_msg == nullptr ? m_val : def;
  }

  template <typename Fn>
    requires(!consumer<Fn, T>)
  [[nodiscard]] constexpr auto then(Fn &&fn) const noexcept
      -> req<decltype(fn(T{}))> {
    if (m_msg == nullptr)
      return {fn(m_val), nullptr};
    return {erred{}, m_msg};
  }
  template <consumer<T> Fn>
  [[nodiscard]] constexpr auto then(Fn &&fn) const noexcept -> req<void> {
    if (m_msg == nullptr) {
      fn(m_val);
      return {};
    }
    return {erred{}, m_msg};
  }
  template <typename S, typename TT = T>
  [[nodiscard]] constexpr req<S> then(S TT::*m) const noexcept {
    return {(m_msg == nullptr) ? m_val.*m : S{}, m_msg};
  }

  [[nodiscard]] constexpr req<T> otherwise(T def) const noexcept {
    return {m_msg == nullptr ? m_val : def, nullptr};
  }
  [[nodiscard]] constexpr req<T> otherwise(auto fn) const noexcept {
    return req<T>{m_msg == nullptr ? m_val : fn(), nullptr};
  }

  [[nodiscard]] constexpr auto compose(auto fn) const noexcept
      -> req<typename decltype(fn(T{}))::type> {
    if (m_msg == nullptr)
      return fn(m_val);
    return {erred{}, m_msg};
  }

  [[nodiscard]] constexpr static req<T> failed(const char *m) noexcept {
    return {T{}, m};
  }
};
export template <typename T> req(T) -> req<T>;

static_assert([] {
  constexpr const auto flip = [](bool b) { return !b; };
  return req<bool>::failed("Some error message")
      .then(flip)
      .then(flip)
      .unwrap(true);
}());
static_assert(req{true}.unwrap(false));

static_assert(req<bool>::failed("Error").otherwise(true).unwrap(false));
static_assert(
    req<bool>::failed("Error").otherwise([] { return true; }).unwrap(false));

static_assert(req{false}.compose([](bool) { return req{true}; }).unwrap(false));

static_assert(
    req{false}.then([](auto) {}).then([] { return true; }).unwrap(false));

static_assert(req<void>{}
                  .then([] {})
                  .otherwise([] { throw 0; })
                  .compose([] { return req<void>{}; })
                  .compose([] { return req{true}; })
                  .unwrap(false));

} // namespace mno
