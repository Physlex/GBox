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
import :lazy;
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
            std::invocable<F, Args...> &&
            std::invocable<G, std::invoke_result_t<F, Args...>>
        )
    constexpr decltype(auto) operator()(Args &&...args) && {
        decltype(auto) fn_res = std::move(this->f_)(std::forward<Args>(args)...);
        return std::move(this->g_)(std::forward<decltype(fn_res)>(fn_res));
    }

    /// Consumes the chain, handing back its first and second stages
    constexpr std::pair<F, G> split() && {
        return {std::move(this->f_), std::move(this->g_)};
    }

  private:
    [[no_unique_address]] F f_;
    [[no_unique_address]] G g_;
};

template <class F, class G>
inline constexpr bool traits::is_chain<Chain<F, G>> = true;

/// Composes two stages into a [`Chain`] that nests on the right.
///
/// A chain given as the first stage is split, and the new stage is composed onto its
/// second, so `(a | b) | c` builds the same shape as `a | (b | c)`.
template <class F, class G>
constexpr auto compose(F f, G g) {
    if constexpr (traits::IsChain<F>) {
        auto [head, tail] = std::move(f).split();
        auto rest = compose(std::move(tail), std::move(g));

        return Chain<decltype(head), decltype(rest)>(std::move(head), std::move(rest));
    } else {
        return Chain<F, G>(std::move(f), std::move(g));
    }
}

namespace traits {

/// Composes two callables into a [`Chain`], provided the first is chainable.
///
/// A lazy first stage makes the whole chain lazy: the laziness is lifted off the first
/// stage and placed around the chain instead.
export template <class F, class G>
    requires(IsChainable<F>)
constexpr auto operator|(F &&f, G &&g) {
    if constexpr (IsLazy<F>) {
        return lazy::Lazy(compose(
            std::decay_t<F>(std::forward<F>(f)).strip(),
            std::decay_t<G>(std::forward<G>(g))
        ));
    } else {
        return compose(
            std::decay_t<F>(std::forward<F>(f)), std::decay_t<G>(std::forward<G>(g))
        );
    }
}

}  // namespace traits

}  // namespace chain
