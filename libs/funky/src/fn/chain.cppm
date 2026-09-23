module;

//! This module implements the chain operand and policies for all funky fn types
//!
//! ## Lazy Promotion
//! Lazy Promotion is the following property:
//!
//! Suppose f is a function, and g is a function.
//! Suppose that it is the case that f is a lazy function.
//!
//! Then given f | g <=> g(f), then => g(Lazy<f>) is a function g which is predicated on
//! a lazy function. Hence, it is also lazy.
//!
//! The other way is also true. Suppose instead that g is a lazy function.
//!
//! Then given f | g <=> g(f), then => Lazy<g>(f) is a function g which has an input f
//! evaluated immediately and returning a lazy operation.
//!
//! Hence in both circumstances, they are lazy.
//!
//! We say a chain is lazy then, if either of these conditions are held. Put another way,
//! a chain is lazy iff either f is lazy, or g is lazy.

#include <concepts>
#include <type_traits>
#include <utility>

export module gbox.funky.fn:chain;

import gbox.core;

import :chain.traits;
import :lazy.traits;
import :partial.traits;

using lazy::traits::IsLazy;
using partial::traits::Bindable;

namespace chain {

using traits::Chainable;

/// The composition of two callables, invoked left to right.
///
/// Reached through `operator|` rather than written directly, since a chain names closure
/// types that cannot be spelled at a call site.
///
/// # Example
/// ```cpp
/// auto add_then_double = once([](int32_t a, int32_t b) { return a + b; })
///     | once([](int32_t sum) { return sum * 2; });
///
/// auto six = std::move(add_then_double)(1, 2);
/// ```
export template <typename F, typename G>
class Chain : public For<Chain<F, G>, Where<Chainable<Self>, Bindable<Self>>> {
  public:
    constexpr Chain(F f, G g) : f_(std::move(f)), g_(std::move(g)) {}

    /// Consumes the chain, invoking F with the arguments given and handing whatever it
    /// returned to G.
    ///
    /// ## Returns
    /// Uses decltype(auto) pattern to "forward" the return type of the evaluated closure
    /// directly to the owning type.
    template <typename... Args>
        requires(
            !IsLazy<F> && !IsLazy<G> && std::invocable<F, Args...> &&
            std::invocable<G, std::invoke_result_t<F, Args...>>
        )
    constexpr decltype(auto) operator()(Args &&...args) && {
        decltype(auto) fn_res = std::move(this->f_)(std::forward<Args>(args)...);
        return std::move(this->g_)(std::forward<decltype(fn_res)>(fn_res));
    }

  private:
    [[no_unique_address]] F f_;
    [[no_unique_address]] G g_;
};

namespace traits {

/// Composes two callables into a [`Chain`], provided both are chainable
export template <class F, class G>
    requires(IsChainable<F>)
constexpr auto operator|(F &&f, G &&g) {
    return Chain<std::decay_t<F>, std::decay_t<G>>(
        std::forward<F>(f), std::forward<G>(g)
    );
}

}  // namespace traits

}  // namespace chain

/// A chain is lazy if either of the two containing types are also lazy
template <class F, class G>
inline constexpr bool lazy::traits::is_lazy<chain::Chain<F, G>> =
    lazy::traits::is_lazy<F> || lazy::traits::is_lazy<G>;
