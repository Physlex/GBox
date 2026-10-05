module;

//! TODO: DOCS

#include <concepts>
#include <utility>

export module gbox.funky.fn:yield;

import gbox.core;

import :yield.traits;

export namespace yield {

/// Represents an abstract "deffered" operation
///
/// It is up to the implementor to decide what exactly "deffered" implies, and how it's
/// orchestrated. The only example implementor of the yield type in the `funky` library is
/// the `Thunk` type, which simply defers execution until yield is invoked.
template <typename R>
class Yield {
  public:
    /// Forces execution to occur
    ///
    /// ## Usage
    /// Depends on the implementor. For most `funky` cases, this simply invokes the lazy
    /// function using the `Thunk` type and forwards the return value.
    constexpr virtual R yield() & noexcept = 0;

    /// Move-only mirror of [`yield`]
    constexpr virtual R yield() && noexcept = 0;
};

/// Any class that publicly derives from `Yield<R>` is yieldable.
template <class T>
    requires std::derived_from<T, Yield<decltype(std::declval<T &>().yield())>>
inline constexpr bool traits::is_yieldable<T> = true;

}  // namespace yield
