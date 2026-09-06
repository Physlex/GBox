module;

//! This module defines the `Partial` object, a move-only layer that holds a callable
//! together with some of the arguments it will eventually be invoked with.
//!
//! Each `bind` supplies one or more arguments and produces a new layer wrapping the
//! previous one, so the type of a chain reads as the order the arguments were supplied
//! in. Accumulating arguments and invoking are separate operations: `bind` only ever
//! produces another layer, and `operator()` only ever invokes.

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

export module gbox.func_ky:func.partial;

import gbox.core;

using memory::MoveOnly;

export namespace partial {

/// One stage of a partial application, holding the previous stage together with the
/// arguments bound at this one.
///
/// `F` is whatever the previous stage produced, either the once-callable that began the
/// chain or the layer beneath this one. `Bound` are the arguments supplied to the `bind`
/// that created this layer.
///
/// # Example
/// ```cpp
/// auto chain = make_once([](int a, int b) { return a + b; }).bind(1).bind(2);
/// // Partial<Partial<OnceFn<C>, int>, int>
/// ```
template <typename F, typename... Bound>
class Partial : public MoveOnly {
  public:
    /// Constructs a layer over the previous stage of the chain.
    ///
    /// Reached through `bind` rather than written directly; a chain names a closure
    /// type, so its own type cannot be spelled at a call site.
    constexpr Partial(F &&f, std::decay_t<Bound>... args)
        : f_(std::move(f)), args_(std::move(args)...) {}

    /// Supplies further arguments, producing a layer that owns them.
    ///
    /// This never invokes, however many arguments have accumulated. Binding and calling
    /// are separate operations, so a chain is only run when it is called.
    ///
    /// # Example
    /// ```cpp
    /// auto chain = make_once([](int a, int b) { return a + b; }).bind(1).bind(2);
    /// ```
    template <typename... New>
    [[nodiscard]] constexpr auto bind(New &&...args) && {
        return Partial<Partial, std::decay_t<New>...>(
            std::move(*this), std::forward<New>(args)...
        );
    }

    /// Consumes the chain, invoking the wrapped callable with every bound argument.
    ///
    /// The arguments bound at this layer are placed ahead of anything the layer above
    /// supplied, so a chain applies its arguments in the order they were bound. Any
    /// arguments given here are appended last and the call happens immediately; this
    /// never produces another layer, which is what `bind` is for.
    ///
    /// Only callable once the wrapped callable can actually accept the accumulated
    /// arguments, so an incomplete chain has no `operator()` at all.
    ///
    /// # Example
    /// ```cpp
    /// auto sum = [](int a, int b, int c) { return a + b + c; };
    ///
    /// make_once(sum).bind(1).bind(2).bind(3)();   // 6
    /// make_once(sum).bind(1, 2)(3);               // 6, last argument given at the call
    /// ```
    template <typename... Extra>
        requires std::invocable<F, std::decay_t<Bound>..., Extra...>
    constexpr decltype(auto) operator()(Extra &&...extra) && {
        return std::apply(
            [&](auto &&...bound) -> decltype(auto) {
                return std::move(this->f_)(
                    std::forward<decltype(bound)>(bound)..., std::forward<Extra>(extra)...
                );
            },
            std::move(this->args_)
        );
    }

  private:
    [[no_unique_address]] F f_;
    [[no_unique_address]] std::tuple<std::decay_t<Bound>...> args_;
};

}  // namespace partial
