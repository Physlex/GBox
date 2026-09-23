module;

//! This module implements the `Lazy` type annotation, which is used to indicate that a
//! function is a deffered execution type. By construction, an N-arity wrapped dependent
//! from `Lazy` is also lazy.
//!
//! ## Example
//!
//! TODO: DOCS

#include <concepts>
#include <functional>
#include <utility>

export module gbox.funky.fn:lazy;

import gbox.core;

import :chain.traits;
import :lazy.traits;
import :thunk;

using chain::traits::Chainable;
using thunk::Thunk;

namespace lazy {

/// Lazyness type-annotation
///
/// Used in funky to ensure that a function is not eagerly evaluated, as oppoesed to
/// the default. It's typically used to enable defference of execution from some known
/// funky source.
///
/// It's also the only way to ensure the underlying type is thunk-able.
export template <typename F>
class Lazy : public For<Lazy<F>, Where<Chainable<Self>>> {
  public:
    constexpr Lazy(F f) : f_(std::forward<F>(f)) {}

    /// Consumes the `Lazy` type and thunks it, erasing the associated type and forwarding
    /// the return type of the underlying storage type.
    template <typename... Args>
    auto operator()(Args... args) &&
        requires(std::invocable<F, Args...>)
    {
        using R = std::invoke_result_t<F, Args...>;

        auto decorator = [fn = std::move(this->f_),
                          ... args = std::forward<Args>(args)]() mutable -> R {
            return std::invoke(std::move(fn), std::move(args)...);
        };

        return Thunk<R>(decorator);
    }

    /// Removes the lazyness attribute and returns the inner type
    auto strip() &&
        requires(!std::invocable<F>)
    {
        return std::move(this->f_);
    }

  private:
    [[no_unique_address]] F f_;
};

template <class F>
inline constexpr bool traits::is_lazy<Lazy<F>> = true;

/// `Lazy` type-annotation factory function.
///
/// Preferred over manual construction of the `Lazy` class.
///
/// ## Idempotent
/// The lazyness factory function is idempotent. Lazy<Lazy<T>> == Lazy<T>.
export template <typename F>
constexpr Lazy<F> lazy(F f) {
    if constexpr (traits::IsLazy<F>) {
        return f;
    } else {
        return Lazy(std::forward<F>(f));
    }
}

}  // namespace lazy
