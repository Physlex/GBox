module;

//! This module defines the `OnceFn` object, a move-only wrapper around a closure which
//! allows for simple ownership of function types.
//!
//! Importantly, it's heapless, meaning it's faster and safer by default.
//!
//! A once-callable is the start of every chain: `OnceFn` wraps a closure, `bind`
//! turns it into a partial application, and calling it consumes it.

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

export module gbox.func_ky:func.once;

import :func.partial;

import gbox.core;

using memory::MoveOnly;
using partial::Partial;

export namespace once_fn {

/// Default closure definition for the No-op closure representation of the `OnceFn`
struct NoOp {
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
class OnceFn : public For<OnceFn<C>, Where<MoveOnly<Self>>> {
  public:
    /// Default constructor for a `OnceFn`
    constexpr OnceFn() : closure_([]() {}) {}

    /// Takes ownership of a callable. Every chain begins here.
    constexpr explicit OnceFn(C closure) : closure_(std::forward<C>(closure)) {}

    /// Binds arguments ahead of the call, producing a partial application.
    ///
    /// # Example
    /// ```cpp
    /// auto add_one_to = OnceFn([](int a, int b) { return a + b; }).partial(1);
    /// auto total = std::move(add_one_to)(2);
    /// ```
    template <typename... Args>
    [[nodiscard]] constexpr auto partial(Args &&...args) && {
        return Partial<OnceFn, std::decay_t<Args>...>(
            std::move(*this), std::forward<Args>(args)...
        );
    }

    /// Binds arguments ahead of the call, producing a thunk.
    ///
    /// # Example
    /// ```cpp
    /// auto thunk = OnceFn([](int a, int b) { return a + b; }).thunk(1, 2);
    /// auto res = std::move(thunk)(); // Invoked at a later time, when the application
    /// deems it valuable.
    /// ```
    [[nodiscard]] constexpr auto thunk() &&
        requires std::invocable<C>
    {
        return Thunk(this->closure_);
    }

    /// Constructor for simplified function composition
    ///
    /// Allows an extremely simplified composition of functions using the pipe operator
    template <typename Self, typename Gn>
    constexpr decltype(auto) operator|(this Self &&self, Gn &&gn) {
        return once_fn::OnceFn(
            [fn = std::forward<Self>(self), gn = std::forward<Gn>(gn)]<typename... Args>(
                Args &&...args
            ) mutable -> decltype(auto) {
                return std::move(gn)(std::move(fn)(std::forward<Args>(args)...));
            }
        );
    }

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

/// Deducation guideline for the basic construction case
OnceFn() -> OnceFn<NoOp>;

/// factory function for the once fn type
///
/// ## Usage
/// Typically used at the start of a function chain sequence, IE:
/// auto pipeline = (once(f) | g | h).thunk();

}  // namespace once_fn
