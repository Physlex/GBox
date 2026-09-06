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
#include <functional>
#include <type_traits>
#include <utility>

export module gbox.func_ky:func.thunk;

import gbox.core;

using memory::MoveOnly;

export namespace thunk {

/// A unit of deferred work returning `R`, taking no arguments and holding no state.
///
/// # Example
/// ```cpp
/// static auto work = make_once([] { return 42; });
///
/// auto pending = make_thunk<work>();
/// auto answer = std::move(pending).call();
/// ```
template <typename R>
class Thunk : public MoveOnly {
  public:
    /// Constructs a thunk with no work attached, so that a container of thunks can be
    /// filled in after it is created. Calling such a thunk is erroneous.
    constexpr Thunk() = default;

    /// Adopts a callable that neither takes arguments nor carries state.
    ///
    /// The conversion a stateless closure offers to a plain function pointer is what
    /// admits it here; one that captures anything has no such conversion and is
    /// rejected.
    template <typename Fn>
        requires std::invocable<Fn> && std::convertible_to<Fn, R (*)(void)>
    constexpr Thunk(Fn &&fn_ptr)
        : fn_ptr_(static_cast<R (*)(void)>(std::forward<Fn>(fn_ptr))) {}

    /// Runs the work.
    constexpr R call() const { return this->fn_ptr_(); }

    /// Runs the work.
    constexpr R operator()() const { return this->call(); }

    /// Reports whether any work is attached.
    [[nodiscard]] constexpr bool is_bound() const { return this->fn_ptr_ != nullptr; }

  private:
    R (*fn_ptr_)(void) = nullptr;
};

/// Erases a fully bound chain into a thunk.
///
/// The chain is named as a template argument rather than passed as one, so the language
/// requires it to have static storage duration; a chain held in a local cannot be given
/// here at all. This is what lets the resulting thunk be nothing but an address: the
/// bound arguments stay where the chain lives, and the thunk outlives the scope it was
/// made in.
///
/// Invoking the thunk consumes the chain it names.
///
/// # Example
/// ```cpp
/// static auto work = make_once([](int pin, bool level) { gpio_write(pin, level); })
///                        .bind(13)
///                        .bind(true);
///
/// queue.push(make_thunk<work>());
/// ```
template <auto &Chain>
    requires std::invocable<std::remove_reference_t<decltype(Chain)>>
[[nodiscard]] constexpr auto make_thunk() {
    using Chained = std::remove_reference_t<decltype(Chain)>;

    return Thunk<std::invoke_result_t<Chained>>{+[]() -> decltype(auto) {
        return std::move(Chain)();
    }};
}

}  // namespace thunk
