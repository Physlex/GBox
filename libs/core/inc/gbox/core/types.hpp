#ifndef GBOX_CORE_TYPES_HPP_
#define GBOX_CORE_TYPES_HPP_

//! This file includes a collection of gbox primitives

#include <stdint.h>

#include <cstddef>

using float32_t = float;
using float64_t = double;

/// An empty function deduction guideline
///
/// Declares the Fn type, with an unspecified signature. Effectively defines a generic
/// function with any kind of argumentation.
template <typename Sig>
struct EmptyFn;

/// Specializes the Fn empty type into a type specified by return type and argument types
template <typename Ret, typename... Args>
struct EmptyFn<Ret(Args...)> {
    using type = Ret (*)(Args...);
};

/// Convenience wrapper, defines a type funcptr_t that should arguably exist in the std
template <typename Sig>
using Fn = typename EmptyFn<Sig>::type;

#endif  // GBOX_CORE_TYPES_HPP_
