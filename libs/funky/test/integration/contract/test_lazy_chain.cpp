//! This file implements testing for the contract between chains and the lazyness
//! annotation: which operand defers, and where in a chain it sits, decides whether the
//! chain that names it defers too.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <cstdint>

import gbox.funky;

using lazy::traits::is_lazy;

namespace {

/// A callable that defers nothing, which is what begins an ordinary chain
constexpr auto add_once() {
    return fn::once([](int32_t a, int32_t b) { return a + b; });
}

/// The same callable under the lazyness annotation, so a chain naming it must promote
constexpr auto add_once_lazy() { return fn::lazy(add_once()); }

}  // namespace

/// A chain of two eager callables has nothing to defer
static_assert(
    !is_lazy<decltype(add_once() | add_once())>, "a chain of two eager callables is eager"
);

/// Either operand defers, and the chain defers with it, whichever side it was given on
static_assert(
    is_lazy<decltype(add_once() | add_once_lazy())>,
    "a chain whose second operand is lazy is lazy"
);
static_assert(
    is_lazy<decltype(add_once_lazy() | add_once())>,
    "a chain whose first operand is lazy is lazy"
);
static_assert(
    is_lazy<decltype(add_once_lazy() | add_once_lazy())>,
    "a chain of two lazy callables is lazy"
);

/// A chain is chainable itself, so the rule has to answer the same way when an operand is
/// a chain rather than a leaf
static_assert(
    is_lazy<decltype(add_once() | (add_once_lazy() | add_once_lazy()))>,
    "an eager callable chained onto a lazy chain is lazy"
);
static_assert(
    is_lazy<decltype(add_once_lazy() | (add_once_lazy() | add_once_lazy()))>,
    "a lazy callable chained onto a lazy chain is lazy"
);
static_assert(
    !is_lazy<decltype(add_once() | (add_once() | add_once()))>,
    "an eager callable chained onto an eager chain is eager"
);
static_assert(
    is_lazy<decltype(add_once_lazy() | (add_once() | add_once()))>,
    "a lazy callable chained onto an eager chain is lazy"
);
