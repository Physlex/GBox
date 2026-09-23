module;

//! This module defines the `OnceFn` object, a move-only wrapper around a closure which
//! allows for simple ownership of function types.
//!
//! Importantly, it's heapless, meaning it's faster and safer by default.
//!
//! A once-callable is the start of every chain: `OnceFn` wraps a closure, `operator<<`
//! binds arguments ahead of the call, and calling it consumes it.

#include <concepts>
#include <type_traits>
#include <utility>

export module gbox.funky.fn:once;

import gbox.core;

import :chain.traits;
import :partial.traits;

using chain::traits::Chainable;
using memory::MoveOnly;
using partial::traits::Bindable;

export namespace once_fn {

/// Default closure definition for the No-op closure representation of the `OnceFn`
struct Noop {
    constexpr void operator()() const noexcept {}
};

/// Sole owner of a callable, consumed by the call that invokes it.
///
/// # Example
/// ```cpp
/// auto sum = OnceFn([](int a, int b) { return a + b; });
/// auto total = std::move(sum)(1, 2);
/// ```
template <typename C>
class OnceFn
    : public For<OnceFn<C>, Where<MoveOnly<Self>, Chainable<Self>, Bindable<Self>>> {
  public:
    /// Default constructor for a `OnceFn`
    constexpr OnceFn() : closure_([]() {}) {}

    /// Takes ownership of a callable. Every chain begins here.
    constexpr explicit OnceFn(C closure) : closure_(std::forward<C>(closure)) {}

    /// Operator overload for the OnceFn type, consuming the type to invoke the inner
    /// closure
    ///
    /// ## Returns
    /// Uses decltype(auto) pattern to "forward" the return type of the evaluated closure
    /// directly to the owning type.
    template <typename... Args>
        requires std::invocable<C, Args...>
    constexpr decltype(auto) operator()(Args &&...args) && {
        return std::move(this->closure_)(std::forward<Args>(args)...);
    }

    /// The non-move mutable operator overloads for the `OnceFn` are deleted to provent
    /// weird compiler errors and make it more obvious that it's intentional behaviour.
    ///
    /// ## Note
    /// Technically, it doesn't really matter that the return type of any of these are
    /// void. C++'s compiler will notice that *any* of these are deleted, and propagate.
    template <typename... Args>
    constexpr void operator()(Args &&...) & = delete;
    template <typename... Args>
    constexpr void operator()(Args &&...) const & = delete;
    template <typename... Args>
    constexpr void operator()(Args &&...) const && = delete;

  private:
    // NOTE: A closure which is constructed without captures is considered "empty", to
    // forward this to the inner type (storage type) we use the `no_unique_address` clang
    // compiler attribute.
    [[no_unique_address]] C closure_;
};

/// Deduction guideline for the basic construction case
OnceFn() -> OnceFn<Noop>;

/// factory function for the once fn type.
///
/// It's not necessary, but keeps the functional inspiration intact.
template <typename F>
constexpr OnceFn<std::decay_t<F>> once(F &&closure) {
    return OnceFn(std::forward<F>(closure));
}

}  // namespace once_fn
