export module missingno:value;
import :traits;

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
template <typename A, typename B>
concept not_same_as = (!is_same<A, B>::value);

template <typename T, typename V>
concept consumer = requires(T t, V &&v) {
                     { t(fwd<V>(v).v) } -> same_as<void>;
                   };
template <typename T, typename V>
concept mapper = requires(T t, V &&v) {
                   { t(fwd<V>(v).v) } -> not_same_as<void>;
                 };
template <typename T>
concept void_consumer = requires(T t) {
                          { t() } -> same_as<void>;
                        };
template <typename T>
concept void_mapper = requires(T t) {
                        { t() } -> not_same_as<void>;
                      };

template <typename T> struct value {
  T v{};

  using type = T;
};
template <> struct value<void> {
  using type = void;
};
template <typename T> value(T) -> value<T>;

template <typename V>
[[nodiscard]] constexpr auto map(V &&v, mapper<V> auto fn) noexcept {
  return value{fn(fwd<V>(v).v)};
}
template <typename V>
[[nodiscard]] constexpr auto map(V &&v, consumer<V> auto fn) noexcept {
  fn(fwd<V>(v).v);
  return value<void>{};
}
[[nodiscard]] constexpr auto map(value<void> v, void_mapper auto fn) noexcept {
  return value{fn()};
}
[[nodiscard]] constexpr auto map(value<void> v,
                                 void_consumer auto fn) noexcept {
  fn();
  return value<void>{};
}
template <typename T>
[[nodiscard]] constexpr bool operator==(const value<T> &a,
                                        const value<T> &b) noexcept {
  return a.v == b.v;
}
[[nodiscard]] constexpr bool operator==(const value<void> &a,
                                        const value<void> &b) noexcept {
  return true;
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
static_assert([] {
  struct s {
    constexpr s() = default;
    constexpr s(const s &) = delete;
    constexpr s &operator=(const s &) = delete;
    constexpr s(s &&) = default;
    constexpr s &operator=(s &&) = default;
    constexpr bool non_const_check() noexcept { return true; }
  };
  value<s> v0{};
  auto v1 = map(v0, [](auto &a) { return a.non_const_check(); });
  return v1.v;
}());
} // namespace mno
