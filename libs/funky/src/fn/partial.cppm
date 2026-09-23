module;

//! This module defines the `Partial` object, a layer that holds a callable together with
//! some of the arguments it will eventually be invoked with.
//!
//! `operator<<` supplies arguments and produces a new layer, so the type of a chain reads
//! as the order the arguments were supplied in. Accumulating arguments and invoking are
//! separate operations: `operator<<` only ever produces another layer, and `operator()`
//! only ever invokes.
//!
//! A single argument nests a layer around the previous one, while a `std::tuple` spreads
//! its elements across one flat layer.

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

export module gbox.funky.fn:partial;

import gbox.core;

import :chain.traits;
import :lazy.traits;
import :partial.traits;

using chain::traits::Chainable;
using lazy::traits::IsLazy;
using partial::traits::Bindable;

export namespace partial {

/// One stage of a partial application, holding the previous stage together with the
/// arguments bound at this one.
///
/// `C` is the stage this layer wraps, either the callable that began the chain or the
/// layer beneath this one. `Bound` are the arguments supplied to the `operator<<` that
/// created this layer.
///
/// # Example
/// ```cpp
/// auto chain = f << 1 << 2;
/// // Partial<Partial<decltype(f), int32_t>, int32_t>
/// ```
template <typename C, typename... Bound>
class Partial : public For<Partial<C, Bound...>, Where<Chainable<Self>, Bindable<Self>>> {
  public:
    /// Constructs a layer over the previous stage of the chain.
    ///
    /// Reached through `operator<<` rather than written directly; a chain names a closure
    /// type, so its own type cannot be spelled at a call site.
    constexpr Partial(C f, std::decay_t<Bound>... args)
        : closure_(std::move(f)), args_(std::move(args)...) {}

    /// Consumes the chain, invoking the wrapped callable with every bound argument.
    ///
    /// The arguments bound at this layer are placed ahead of anything the layer above
    /// supplied, so a chain applies its arguments in the order they were bound. Any
    /// arguments given here are appended last and the call happens immediately; this
    /// never produces another layer, which is what `operator<<` is for.
    ///
    /// Only callable once the wrapped callable can actually accept the accumulated
    /// arguments, so an incomplete chain has no `operator()` at all.
    ///
    /// # Example
    /// ```cpp
    /// // f wraps [](int32_t a, int32_t b, int32_t c) { return a + b + c; }
    ///
    /// std::move(f << 1 << 2 << 3)();                 // 6
    /// std::move(f << std::tuple{1, 2})(3);           // 6, last argument given at the
    /// call
    /// ```
    template <typename... Args>
        requires(!IsLazy<C> && std::invocable<C, std::decay_t<Bound>..., Args...>)
    constexpr decltype(auto) operator()(Args &&...args) && {
        return std::apply(
            [&](auto &&...bound) -> decltype(auto) {
                return std::move(this->closure_)(
                    std::forward<decltype(bound)>(bound)..., std::forward<Args>(args)...
                );
            },
            std::move(this->args_)
        );
    }

    /// Invokes the wrapped callable, leaving the layer callable again afterwards.
    ///
    /// Available when the wrapped callable is invocable as an lvalue and takes the bound
    /// arguments without consuming them, since the layer keeps owning them.
    template <typename... Args>
        requires(!IsLazy<C> && std::invocable<C &, std::decay_t<Bound> &..., Args...>)
    constexpr decltype(auto) operator()(Args &&...args) & {
        return std::apply(
            [&](auto &...bound) -> decltype(auto) {
                return this->closure_(bound..., std::forward<Args>(args)...);
            },
            this->args_
        );
    }

  private:
    [[no_unique_address]] C closure_;
    [[no_unique_address]] std::tuple<std::decay_t<Bound>...> args_;
};

namespace traits {

/// Binds a single argument ahead of the call, producing a layer that owns it.
///
/// This never invokes, however many arguments have accumulated. Binding and calling are
/// separate operations, so a chain is only run when it is called.
///
/// # Example
/// ```cpp
/// auto layered = f << 1 << 2;
/// // Partial<Partial<decltype(f), int32_t>, int32_t>
/// ```
template <class F, class A>
    requires(IsBindable<F>)
[[nodiscard]] constexpr auto operator<<(F &&f, A &&arg) {
    return Partial<std::decay_t<F>, std::decay_t<A>>(
        std::forward<F>(f), std::forward<A>(arg)
    );
}

/// Binds every element of a tuple ahead of the call, producing one flat layer that owns
/// them all.
///
/// A tuple is always spread across separate bindings, so the two spellings differ in the
/// shape they build rather than in what they later invoke.
///
/// ## Warning
/// Binding a tuple as a single argument needs the wrapping tuple spelled out in full.
/// `std::tuple{t}` does not do it: class template argument deduction prefers the copy
/// deduction candidate and hands back `t` itself, which then spreads. Write
/// `std::tuple<std::tuple<int32_t, int32_t>>{t}` instead.
///
/// # Example
/// ```cpp
/// auto flat = f << std::tuple{1, 2};
/// // Partial<decltype(f), int32_t, int32_t>
/// ```
template <class F, class... Args>
    requires(IsBindable<F>)
[[nodiscard]] constexpr auto operator<<(F &&f, std::tuple<Args...> args) {
    return std::apply(
        [&f](auto &&...bound) {
            return Partial<std::decay_t<F>, std::decay_t<Args>...>(
                std::forward<F>(f), std::forward<decltype(bound)>(bound)...
            );
        },
        std::move(args)
    );
}

}  // namespace traits

}  // namespace partial
