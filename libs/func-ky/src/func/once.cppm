module;

//! This module defines the `OnceFn` object, a move-only wrapper around a lambda which
//! allows for simple ownership of function types.
//!
//! Importantly, it's heapless, meaning it's faster and safer by default.
//!
//! A once-callable is the start of every chain: `make_once` wraps a closure, `bind`
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

export namespace once {

/// Sole owner of a callable, consumed by the call that invokes it.
///
/// # Example
/// ```cpp
/// auto sum = make_once([](int a, int b) { return a + b; });
/// auto total = std::move(sum)(1, 2);
/// ```
template <typename Fn>
class OnceFn : public MoveOnly {
  public:
    /// Takes ownership of a callable. Reached through `make_once`.
    constexpr explicit OnceFn(Fn lambda) : lambda_(std::move(lambda)) {}

    /// Binds arguments ahead of the call, producing a partial application.
    ///
    /// # Example
    /// ```cpp
    /// auto chain = make_once([](int a, int b) { return a + b; }).bind(1);
    /// auto total = std::move(chain)(2);
    /// ```
    template <typename... Args>
    [[nodiscard]] constexpr auto bind(Args &&...args) && {
        return Partial<OnceFn, std::decay_t<Args>...>(
            std::move(*this), std::forward<Args>(args)...
        );
    }

    /// Operator overload for the OnceFn type, consuming the type to invoke the inner
    /// lambda
    ///
    /// ## Returns
    /// Uses decltype(auto) pattern to "forward" the return type of the evaluated lambda
    /// directly to the owning type.
    template <typename... Args>
        requires std::invocable<Fn, Args...>
    constexpr decltype(auto) operator()(Args &&...args) && {
        return std::invoke(std::move(this->lambda_), std::forward<Args>(args)...);
    }

  private:
    [[no_unique_address]] Fn lambda_;
};

/// Wraps a callable so that it is owned, moved rather than copied, and spent when it is
/// called. Every chain begins here.
template <typename Fn>
[[nodiscard]] constexpr auto make_once(Fn &&f) {
    return OnceFn<std::decay_t<Fn>>(std::forward<Fn>(f));
}

}  // namespace once
