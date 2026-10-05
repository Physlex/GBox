//! This file implements testing for the contract between chains and the lazyness
//! annotation: which operand defers, and where in a chain it sits, decides whether the
//! chain that names it defers too.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <concepts>
#include <cstdint>
#include <type_traits>

#include "lib.hpp"

import gbox.funky;

using fn::Chain;
using fn::lazily;
using fn::Lazy;
using fn::once;
using lazy::traits::is_lazy;

namespace {

/// A callable that defers nothing, which is what begins an ordinary chain
constexpr auto add_once() {
    return fn::once([](int32_t a, int32_t b) { return a + b; });
}

/// The same callable under the lazyness annotation, so a chain naming it must promote
constexpr auto add_once_lazy() { return fn::lazily(add_once()); }

/// Addition followed by a deferred shift: the first stage runs at the call
using RightLazy = decltype(once(add) | lazily(once(shift)));

/// Deferred addition followed by a shift: nothing runs at the call
using LeftLazy = decltype(lazily(once(add)) | once(shift));

/// Both stages deferred
using BothLazy = decltype(lazily(once(add)) | lazily(once(shift)));

using Add = std::decay_t<decltype(add)>;
using Shift = std::decay_t<decltype(shift)>;

/// Deferred addition followed by two shifts, composed without parentheses
using LazyThenTwoShifts = decltype(lazily(add) | shift | shift);

}  // namespace

/// A chain of two eager callables has nothing to defer
static_assert(
    !is_lazy<decltype(add_once() | add_once())>, "a chain of two eager callables is eager"
);

/// Only the first operand decides whether the chain defers: a lazy second operand is
/// reached with all of its arguments, so it hands back its own work when the chain runs
static_assert(
    !is_lazy<decltype(add_once() | add_once_lazy())>,
    "a chain whose only lazy operand is its second is eager"
);
static_assert(
    is_lazy<decltype(add_once_lazy() | add_once())>,
    "a chain whose first operand is lazy is lazy"
);
static_assert(
    is_lazy<decltype(add_once_lazy() | add_once_lazy())>,
    "a chain of two lazy callables is lazy"
);

/// However many stages a chain has, only its first stage decides whether it defers
static_assert(
    !is_lazy<decltype(add_once() | add_once_lazy() | add_once_lazy())>,
    "a longer chain beginning eagerly is eager, whatever follows"
);
static_assert(
    is_lazy<decltype(add_once_lazy() | add_once_lazy() | add_once_lazy())>,
    "a longer chain of lazy stages is lazy"
);
static_assert(
    !is_lazy<decltype(add_once() | add_once() | add_once())>,
    "a longer chain of eager stages is eager"
);
static_assert(
    is_lazy<decltype(add_once_lazy() | add_once() | add_once())>,
    "a longer chain beginning lazily is lazy, whatever follows"
);

/// A lazy chain is called with the arguments of its first stage, and hands back work in
/// place of the result, whichever of its stages defers
static_assert(
    std::invocable<RightLazy, int32_t, int32_t>,
    "a chain whose second stage is lazy is called with its first stage's arguments"
);
static_assert(
    !std::same_as<std::invoke_result_t<RightLazy, int32_t, int32_t>, int32_t>,
    "a chain whose second stage is lazy hands back work rather than the result"
);

static_assert(
    std::invocable<LeftLazy, int32_t, int32_t>,
    "a chain whose first stage is lazy is called with its first stage's arguments"
);
static_assert(
    !std::same_as<std::invoke_result_t<LeftLazy, int32_t, int32_t>, int32_t>,
    "a chain whose first stage is lazy hands back work rather than the result"
);

static_assert(
    std::invocable<BothLazy, int32_t, int32_t>,
    "a chain of two lazy stages is called with its first stage's arguments"
);
static_assert(
    !std::same_as<std::invoke_result_t<BothLazy, int32_t, int32_t>, int32_t>,
    "a chain of two lazy stages hands back work rather than the result"
);

/// A lazy first stage makes the whole chain lazy work over an eager chain
static_assert(
    std::same_as<decltype(lazily(add) | shift), Lazy<Chain<Add, Shift>>>,
    "a chain whose first stage is lazy is lazy work over the eager chain"
);
static_assert(
    std::same_as<decltype(lazily(add) | lazily(shift)), Lazy<Chain<Add, Lazy<Shift>>>>,
    "a chain of two lazy stages is lazy work over a chain ending in lazy work"
);

/// However many stages follow a lazy first stage, the chain beneath stays nested on the
/// right
static_assert(
    std::same_as<LazyThenTwoShifts, Lazy<Chain<Add, Chain<Shift, Shift>>>>,
    "stages composed after a lazy stage nest on the right beneath the laziness"
);
static_assert(
    std::invocable<LazyThenTwoShifts, int32_t, int32_t>,
    "a longer chain beginning lazily is called with its first stage's arguments"
);
static_assert(
    !std::same_as<std::invoke_result_t<LazyThenTwoShifts, int32_t, int32_t>, int32_t>,
    "a longer chain beginning lazily hands back work rather than the result"
);
