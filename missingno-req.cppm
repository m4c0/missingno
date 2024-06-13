export module missingno:req;
import :value;
import jute;
import no;
import silog;
import traits;

using namespace traits;

namespace mno {
class erred {};

export template <typename T> class req;

export template <typename T> class req {
  value<T> m_val{};
  jute::heap m_msg{};

  constexpr req(value<T> val, jute::heap msg) noexcept
      : m_val{val}
      , m_msg{msg} {}
  constexpr req(erred, jute::heap msg) noexcept : m_val{}, m_msg{msg} {}

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

  // Unfortunately needed if we need to avoid recursive calls
  [[nodiscard]] constexpr bool is_valid() const noexcept {
    return m_msg == jute::heap{};
  }

  [[nodiscard]] constexpr static req<T> failed(jute::view m) noexcept {
    // TODO: avoid copy if view is not from a heap
    return {erred{}, jute::heap{} + m};
  }
  [[nodiscard]] constexpr static req<T> failed(jute::heap m) noexcept {
    return {erred{}, m};
  }
  [[nodiscard]] constexpr req<T> if_failed(jute::view m) const noexcept {
    return is_valid() ? req<T>{m_val} : failed(m);
  }
  [[nodiscard]] constexpr req<T>
  if_failed(traits::is_callable<jute::view> auto fn) const noexcept {
    return is_valid() ? req<T>{m_val} : fn(*m_msg);
  }
  [[nodiscard]] constexpr auto assert(auto fn, jute::view m) noexcept {
    if (!is_valid())
      return req<T>{erred{}, m_msg};
    if (!mno::map(m_val, fn).v)
      // TODO: avoid copy if view is not from a heap
      return req<T>{erred{}, jute::heap{} + m};
    return traits::move(*this);
  }

  [[nodiscard]] constexpr auto trace(jute::view m) noexcept {
    if (is_valid())
      return traits::move(*this);

    return req<T>{erred{}, m_msg + "\n\twhile " + m};
  }
  [[nodiscard]] constexpr auto trace(jute::heap m) noexcept {
    return trace(*m);
  }

  [[nodiscard]] constexpr auto
  take(traits::is_callable<jute::view> auto errfn) {
    if (!is_valid()) {
      errfn(*m_msg);
    }
    return move_out(m_val);
  }
  [[nodiscard]] auto log_error() {
    return take([](auto err) {
      silog::log(silog::error, "%.*s", static_cast<unsigned>(err.size()),
                 err.data());
    });
  }
  [[nodiscard]] auto log_error(traits::is_callable<> auto then) {
    return take([&](auto err) {
      silog::log(silog::error, "%.*s", static_cast<unsigned>(err.size()),
                 err.data());
      then();
    });
  }

  template <typename TT>
    requires is_assignable_from<T, TT>
  [[nodiscard]] constexpr T unwrap(TT def) const noexcept {
    return is_valid() ? m_val.v : def;
  }
  template <typename TT>
    requires is_not_assignable_from<T, TT> && is_same_v<T, call_result_t<TT>>
  [[nodiscard]] constexpr T unwrap(TT def) const noexcept {
    return is_valid() ? m_val.v : def();
  }
  template <typename TT>
  [[nodiscard]] constexpr req<T> otherwise(TT def) const noexcept {
    return req<T>{value<T>{is_valid() ? m_val.v : def}};
  }

  [[nodiscard]] constexpr auto map(auto fn) noexcept {
    using O = typename decltype(mno::map(m_val, fn))::type;

    if (is_valid())
      return req<O>{mno::map(m_val, fn)};
    return req<O>{erred{}, m_msg};
  }
  [[nodiscard]] constexpr auto map(auto fn) const noexcept {
    using O = typename decltype(mno::map(m_val, fn))::type;

    if (is_valid())
      return req<O>{mno::map(m_val, fn)};
    return req<O>{erred{}, m_msg};
  }

  [[nodiscard]] constexpr auto fmap(auto fn) noexcept {
    using RO = typename decltype(mno::map(m_val, fn))::type;
    using O = typename RO::type;

    if (is_valid())
      return mno::map(m_val, fn).v;
    return req<O>{erred{}, m_msg};
  }
  [[nodiscard]] constexpr auto fmap(auto fn) const noexcept {
    using RO = typename decltype(mno::map(m_val, fn))::type;
    using O = typename RO::type;

    if (is_valid())
      return mno::map(m_val, fn).v;
    return req<O>{erred{}, m_msg};
  }

  [[nodiscard]] constexpr auto until_failure(auto fn,
                                             auto fail_check) const noexcept {
    if (!is_valid())
      return fail_check(*m_msg) ? req<T>{erred{}, m_msg} : req<T>{};

    req<T> old = *this;
    req<T> res = old.fmap(fn);
    while (res.is_valid()) {
      old = res;
      res = old.fmap(fn);
    }
    return fail_check(*res.m_msg) ? res : old;
  }
  [[nodiscard]] constexpr auto until_failure(auto fn,
                                             auto fail_check) noexcept {
    if (!is_valid())
      return fail_check(*m_msg) ? req<T>{erred{}, m_msg} : req<T>{};

    req<T> old = traits::move(*this);
    req<T> res = old.fmap(fn);
    while (res.is_valid()) {
      old = traits::move(res);
      res = old.fmap(fn);
    }
    return fail_check(*res.m_msg) ? traits::move(res) : traits::move(old);
  }

  [[nodiscard]] constexpr req<T>
  otherwise(void_consumer auto fn) const noexcept {
    if (is_valid())
      return req<T>{m_val};

    return req<T>{mno::map(value<void>{}, fn)};
  }

  [[nodiscard]] constexpr bool operator==(const req<T> &o) const noexcept {
    if (!is_valid() && !o.is_valid())
      return true;
    if (is_valid() != o.is_valid())
      return false;

    return m_val == o.m_val;
  }
  template <typename TT>
  [[nodiscard]] constexpr bool operator==(TT v) const noexcept {
    return is_valid() && m_val.v == v;
  }
};
template <typename T> req(T) -> req<T>;

static_assert(req{3} == req{3});
static_assert(req{2} != req{3});
static_assert(req{3} != req<int>::failed(""));
static_assert(req<int>::failed("") != req{3});
// TODO: static_assert(req<int>::failed("") != req<int>::failed("a"));
static_assert(req<int>::failed("a") == req<int>::failed("a"));
static_assert(req<int>::failed(jute::heap{} + "a") == req<int>::failed("a"));

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

static_assert(req{3}.trace("testing") == req{3});
static_assert(req<jute::view>::failed("fail")
                  .trace("testing")
                  .if_failed([](jute::view msg) { return req{msg}; })
                  .unwrap(jute::view{}) ==
              req<jute::view>{"fail\n\twhile testing"});

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
[[nodiscard]] constexpr auto combine(traits::is_callable<A, B...> auto fn,
                                     const req<A> &a,
                                     const req<B> &...b) noexcept {
  return a.fmap([&](auto &&aa) {
    return combine(
        [&](auto &&...bb) {
          return fn(traits::fwd<decltype(aa)>(aa),
                    traits::fwd<decltype(bb)>(bb)...);
        },
        b...);
  });
}
export template <typename A>
[[nodiscard]] constexpr auto combine(traits::is_callable<A> auto fn,
                                     const req<A> &a) noexcept {
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

static_assert(mno::req{3}
                  .until_failure(
                      [](auto n) {
                        return n > 0 ? req{n - 1} : req<int>::failed("ok");
                      },
                      [](auto err) { return err != "ok"; })
                  .map([](auto k) { return k == 0; })
                  .unwrap(false));
static_assert(mno::req{3}
                  .until_failure([](auto) { return req<int>::failed("ok"); },
                                 [](auto err) { return false; })
                  .map([](auto k) { return k == 3; })
                  .unwrap(false));
static_assert(mno::req{3}
                  .until_failure([](auto) { return req<int>::failed("nok"); },
                                 [](auto err) { return true; })
                  .map([](auto) { return false; })
                  .if_failed([](auto err) { return req{err == "nok"}; })
                  .unwrap(false));

static_assert([] {
  struct s : no::copy {
    constexpr s() = default;
    constexpr bool non_const() { return true; }
  };
  return req<s>()
      .assert([](auto &&o) { return true; }, "failed")
      .fmap([](auto &&o) { return req{static_cast<s &&>(o)}; })
      .map([](auto &&o) { return o.non_const(); })
      .unwrap(false);
}());
static_assert([] {
  struct s : no::copy {
    bool ok = true;
    constexpr s() = default;
    constexpr s(bool x) : ok{x} {}
  };
  return req<s>()
      .until_failure(
          [](auto &&o) { return o.ok ? req<s>(s{false}) : req<s>::failed(""); },
          [](auto msg) { return false; })
      .map([](auto &&o) { return true; })
      .unwrap(false);
}());
static_assert([] {
  mno::req<no::copy> a{};
  mno::req<no::copy> b{};
  return combine([](auto &&aa, auto &&bb) { return true; }, traits::move(a),
                 traits::move(b))
      .unwrap(false);
}());
} // namespace mno
