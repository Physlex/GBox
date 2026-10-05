module;

//! This module defines the traits used in the yield module

#include <concepts>
#include <type_traits>
#include <utility>

export module gbox.funky.fn:yield.traits;

export namespace yield::traits {
/// type-trait variant of the `is_yieldable` constraint
template <class T>
inline constexpr bool is_yieldable = false;

/// Whether a type can be yielded
template <class F>
concept IsYieldable = is_yieldable<std::remove_cvref_t<F>>;

/// Denotes a type which implements the Yield API
///
/// ## Usage
///
/// When used with the `auto` keyword, allows a consumer to ignore the closure type
/// associated with the expression. That is:
///
/// ```cpp
///
/// constexpr auto work = []() -> int32_t {
///     return 1 + 1 = 2;
/// };
///
/// void foo() {
///     // Equivalent to "construct a deffered move-only work function"
///     YieldImpl<int32_t> auto yieldable = lazily(once(work))();
///     int32_t res = std::move(yieldable).yield(); // Yield in-place and return the res
/// }
/// ```
template <class F, typename R>
concept YieldImpl = IsYieldable<F> && requires(F f) {
    { std::move(f).yield() } -> std::same_as<R>;
};
}  // namespace yield::traits
