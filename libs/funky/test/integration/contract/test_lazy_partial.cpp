//! This file implements testing for the contract between the lazyness annotation and
//! partial application: binding arguments to lazy work keeps it lazy, and once every
//! argument is present a lazy layer hands back work rather than running.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>

import gbox.funky;

using fn::lazily;
using fn::Lazy;
using fn::Partial;
using lazy::traits::is_lazy;

namespace {

constexpr int32_t LHS = 4;
constexpr int32_t RHS = 5;

constexpr auto add = [](int32_t a, int32_t b) -> int32_t { return a + b; };

using Add = std::decay_t<decltype(add)>;

/// Lazy addition with its first argument bound
using LazyHalf = decltype(lazily(add) << LHS);

/// Lazy addition with both arguments bound
using LazyFull = decltype(lazily(add) << LHS << RHS);

/// Lazy addition with both arguments bound in one flat layer
using LazyFlat = decltype(lazily(add) << std::tuple{LHS, RHS});

}  // namespace

/// Binding onto lazy work binds beneath the laziness, so the result is lazy work over an
/// eager layer
static_assert(
    std::same_as<LazyHalf, Lazy<Partial<Add, int32_t>>>,
    "binding an argument to lazy work binds it beneath the laziness"
);
static_assert(
    std::same_as<LazyFull, Lazy<Partial<Partial<Add, int32_t>, int32_t>>>,
    "every layer bound onto lazy work nests beneath the laziness"
);
static_assert(
    std::same_as<LazyFlat, Lazy<Partial<Add, int32_t, int32_t>>>,
    "a flat layer bound onto lazy work sits beneath the laziness"
);

static_assert(is_lazy<LazyHalf>, "binding an argument to lazy work keeps it lazy");
static_assert(is_lazy<LazyFull>, "every layer bound over lazy work stays lazy");
static_assert(is_lazy<LazyFlat>, "a flat layer bound over lazy work stays lazy");

static_assert(
    std::invocable<LazyHalf, int32_t>,
    "a lazy layer can be completed with its last argument at the call"
);
static_assert(
    !std::same_as<std::invoke_result_t<LazyHalf, int32_t>, int32_t>,
    "a lazy layer completed at the call hands back work rather than the result"
);

static_assert(std::invocable<LazyFull>, "a fully bound lazy layer can be called");
static_assert(
    !std::same_as<std::invoke_result_t<LazyFull>, int32_t>,
    "a fully bound lazy layer hands back work rather than the result"
);

static_assert(
    !std::invocable<LazyHalf>,
    "a lazy layer missing an argument cannot be called, the same as an eager one"
);
