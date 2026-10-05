module;

//! This module defines the `Thunk` object, a unit of deferred work that has forgotten
//! what it was made from.
//!
//! A thunk takes no arguments and carries no state of its own; it holds nothing but the
//! address of the code that runs the work. Anything it runs therefore has to live
//! somewhere with a fixed address, which `make_thunk` requires by naming the chain as a
//! template argument rather than taking it as a parameter.
//!
//! Because the type of a thunk says nothing about what produced it, thunks built from
//! unrelated callables share one type and can sit together in a single container.
//!
//! TODO: DOCS, UPDATE

#include <concepts>
#include <utility>

export module gbox.funky.fn:yield.thunk;

import gbox.core;

import :yield;
import :yield.traits;

export namespace yield::thunk {

/// Storage type for `funky`s "basic" [`Yield`] type
///
/// Executes a closure of "work" once requested to `yield`. Type-annotated, meaning all
/// dependent types are defined through an N-arity `F` dependent type.
///
/// For sufficiently complicated types, it may be the case that the `F` type is
/// exceptionally long, and unnameable by the operator (due to an anonymous struct
/// generation via closures).
///
/// ## Yield
/// Implements the `Yield` mixin for this purpose, enabling type-erasure into a [`Yield`]
/// dynamic type via reference, such that consumers need not worry about the storage type.
///
/// ## Storage
/// It is potentially the case that a user would wish to store a Thunk type for some
/// purpose in their codebase. For example:
///
/// ```cpp
/// template<typename F>
/// void foo(Thunk<F, void> thunk) {
///     return thunk.yield();
/// }
/// ```
///
/// Although this compiles fine, the preffered way of managing `Thunk`s (and any yieldable
/// for that matter) is through the impl API:
///
/// ```cpp
/// void foo(YieldImpl<void> auto yieldable) {
///     return yieldable.yield();
/// }
/// ```
///
/// To avoid the need for explicit naming of the closure type. These two forms are
/// equivalent, though it's this author's opinion that the bottom form is cleaner and less
/// noisy.
///
/// ## Storage
/// Because the inner type is anonymous, the size of the `Thunk is partially dependent on
/// the size of the closure, which is fully dependent on it's capture clause(s). As a
/// result, it's highly reccomended that this storage type be passed around via reference
/// to the [`Yield`] type (if caller allows for dynamics), or via the `YieldImpl` helper
/// if the caller prefers generics and static dispatch instead.
template <typename F, typename R>
    requires(std::invocable<F>)
class Thunk final : public For<Thunk<F, R>, MixIn<Yield<R>>> {
  public:
    constexpr Thunk(F fn) : work_(std::forward<F>(fn)) {}

    /// Immediately execute this unit of work, returning the result of the work execution.
    constexpr R yield() & noexcept override { return this->work_(); }
    constexpr R yield() && noexcept override { return std::move(this->work_)(); }

  private:
    [[no_unique_address]] F work_;
};

}  // namespace yield::thunk
