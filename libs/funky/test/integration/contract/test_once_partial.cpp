//! This file implements testing for the contract between the once-callable and the
//! layers that bind its arguments: a layer takes its form from whatever it was built
//! over, and stands as a stage of a chain once it has one.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>

import gbox.funky;

using fn::Partial;

namespace {

/// obvious addition example that basically everyone uses by default
constexpr auto add_once() {
    return fn::once([](int32_t a, int32_t b) -> int32_t { return a + b; });
}

/// The same doubling the chain contracts use, as the callable that begins a composition
constexpr auto shift_once() {
    return fn::once([](int32_t value) -> int32_t { return value << 1; });
}

/// The doubling again as a plain callable, for the far side of a composition
constexpr auto shift = [](int32_t value) -> int32_t { return value << 1; };

using Add = decltype(add_once());

}  // namespace

/// A layer owns its callable the way that callable is owned, so binding a once-callable
/// leaves the layer move-only without the layer saying so.
static_assert(
    std::is_move_constructible_v<Partial<Add, int32_t>>,
    "a layer over a once-callable moves"
);
static_assert(
    !std::is_copy_constructible_v<Partial<Add, int32_t>>,
    "a layer over a once-callable does not copy"
);

/// Consuming the callable is likewise the callable's rule, not the layer's
static_assert(
    std::invocable<Partial<Add, int32_t> &&, int32_t>,
    "a layer over a once-callable is invocable as an rvalue"
);
static_assert(
    !std::invocable<Partial<Add, int32_t> &, int32_t>,
    "a layer over a once-callable is not invocable as an lvalue"
);

/// A layer standing as a stage of a chain takes the arguments it was bound with ahead of
/// whatever the preceding stage handed it.
static_assert(
    (shift_once() | add_once() << 1)(1) == 3,
    "a bound layer receives the preceding stage's result after its own bound arguments"
);

/// A chain is bindable in its own right, so a tuple saturates the stage it begins with
static_assert(
    ((add_once() | shift) << std::tuple{1, 1})() == 4,
    "binding a tuple to a chain supplies every argument its first stage takes"
);
