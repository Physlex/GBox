//! This file implements testing for the contract between the `Thunk` type and the `Yield`
//! it implements.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <concepts>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "lib.hpp"

import gbox.funky;

using fn::Thunk;
using yield::Yield;
using yield::traits::YieldImpl;

/// Every thunk implements the `Yield` of its return type, which is what lets it be
/// referred to without naming its closure
static_assert(std::derived_from<Thunk<Constant, int32_t>, Yield<int32_t>>);
static_assert(std::derived_from<Thunk<Empty, void>, Yield<void>>);
static_assert(std::is_convertible_v<Thunk<Captured, int32_t> &, Yield<int32_t> &>);

/// Yielding hands back exactly what the work returns

static_assert(
    std::same_as<decltype(std::declval<Thunk<Constant, int32_t> &>().yield()), int32_t>
);

static_assert(
    std::same_as<decltype(std::declval<Thunk<Constant, int32_t> &&>().yield()), int32_t>
);

/// Yielding a thunk never throws
static_assert(noexcept(std::declval<Thunk<Constant, int32_t> &>().yield()));
static_assert(noexcept(std::declval<Thunk<Constant, int32_t> &&>().yield()));

/// A thunk satisfies the impl form of `Yield`, so it can be held as `YieldImpl<R> auto`
static_assert(YieldImpl<Thunk<Constant, int32_t>, int32_t>);
