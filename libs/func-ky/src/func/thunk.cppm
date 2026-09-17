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

#include <concepts>
#include <utility>

export module gbox.func_ky:func.thunk;

import gbox.core;

import :option;

using option::None;
using option::Option;

export namespace thunk {

/// A unit of deferred work returning `R`, taking no arguments and holding no state.
///
/// # Example
/// ```cpp
/// static auto work = OnceFn([] { return 42; });
///
/// auto pending = make_thunk<work>();
/// auto answer = std::move(pending).call();
/// ```
template <typename R>
class Thunk {
  private:
    /// Inner type used to refer to the function pointer associated with the thunk
    using Storage = R (*)(void);

  public:
    /// Constructs a thunk with no work attached, so that a container of thunks can be
    /// filled in after it is created. Calling such a thunk is erroneous.
    constexpr Thunk() : fn_ptr_opt_([]() {}) {}

    /// Adopts a callable that neither takes arguments nor carries state.
    ///
    /// The conversion a stateless closure offers to a plain function pointer is what
    /// admits it here; one that captures anything has no such conversion and is
    /// rejected.
    template <typename Fn>
        requires(std::invocable<Fn> && std::convertible_to<Fn, Storage>)
    constexpr Thunk(Fn &&fn_ptr)
        : fn_ptr_opt_(static_cast<Storage>(std::forward<Fn>(fn_ptr))) {}

    /// Invoke the thunk, returning whatever R is collected from the return type
    constexpr decltype(auto) operator()() const noexcept {
        return std::forward<R>(this->fn_ptr_opt_());
    }

  private:
    Storage fn_ptr_opt_;
};

}  // namespace thunk
