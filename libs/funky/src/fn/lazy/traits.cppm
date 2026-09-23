module;

//! This module implements the type traits used to orchestrate the func module.

#include <type_traits>

export module gbox.funky.fn:lazy.traits;

export namespace lazy::traits {

/// The [`is_lazy`] type trait is used to check whether an underlying function type `F`,
/// is Lazy. For funky types, this is used to indicate deffered execution for otherwise-
/// eager types, such as the `AnyFn` types.
template <class T>
inline constexpr bool is_lazy = false;

/// Concept form for lazy evaluation compile-time check
template <class F>
concept IsLazy = is_lazy<std::remove_cvref_t<F>>;

}  // namespace lazy::traits
