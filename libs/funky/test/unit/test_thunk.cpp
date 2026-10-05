//! This file implements testing for the `Thunk` type, the storage type behind `funky`'s
//! basic `Yield` implementation.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <concepts>
#include <cstdint>
#include <type_traits>

#include "lib.hpp"

import gbox.funky;

using fn::Thunk;
using yield::Yield;

namespace {

/// Whether `Thunk<F, R>` can be named at all, so that a rejected closure can be checked
/// without the check itself failing to compile
template <class F, class R>
concept Thunkable = requires { typename Thunk<F, R>; };

}  // namespace

/// A thunk is work that is already fully applied, so a closure still waiting on a
/// parameter cannot be stored as one
static_assert(Thunkable<Empty, void>);
static_assert(Thunkable<Constant, int32_t>);
static_assert(!Thunkable<Parameterised, int32_t>);

/// Unlike a function pointer, a thunk owns its closure, so captured state is admitted
static_assert(std::is_constructible_v<Thunk<Captured, int32_t>, Captured>);

/// A thunk knows the closure it was made from; two different closures make two different
/// thunks even when they return the same type
static_assert(!std::same_as<Thunk<Constant, int32_t>, Thunk<Captured, int32_t>>);

/// A thunk is exactly its `Yield` base plus its closure, adding no overhead of its own
static_assert(sizeof(Thunk<Empty, void>) == sizeof(Yield<void>));
static_assert(sizeof(Thunk<Wide, int64_t>) == sizeof(Yield<int64_t>) + sizeof(Wide));
