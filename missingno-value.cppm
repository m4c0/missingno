export module missingno:value;

namespace mno {
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

template <typename T> struct value {
  T v{};

  using type = T;
};
template <> struct value<void> {
  using type = void;
};
template <typename T> value(T) -> value<T>;

template <typename T>
[[nodiscard]] constexpr auto map(value<T> v, auto fn) noexcept {
  return value{fn(v.v)};
}
template <typename T>
[[nodiscard]] constexpr value<void> map(value<T> v,
                                        consumer<T> auto fn) noexcept {
  fn(v.v);
  return value<void>{};
}
[[nodiscard]] constexpr auto map(value<void> v, auto fn) noexcept {
  return value{fn()};
}
[[nodiscard]] constexpr auto map(value<void> v,
                                 void_consumer auto fn) noexcept {
  fn();
  return value<void>{};
}

static_assert([] {
  // void -> 1 -> bool -> void -> void -> bool
  constexpr const value<void> v0{};
  constexpr const auto va = map(v0, [] { return 1; });
  constexpr const auto vb = map(va, [](auto a) { return a == 1; });
  static_assert(vb.v);
  constexpr const auto v1 = map(vb, [](auto b) {});
  constexpr const auto v2 = map(v1, [] {});
  return map(v2, [] { return true; }).v;
}());
} // namespace mno
