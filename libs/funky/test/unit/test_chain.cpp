//! This file implements unit testing for the `Chain` type: a chain is called with the
//! arguments of its first stage and hands that stage's result to the second.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <concepts>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "lib.hpp"

import gbox.funky;

using fn::Chain;

namespace {

constexpr int32_t LHS = 4;
constexpr int32_t RHS = 5;
constexpr int32_t EXPECTED_RESULT = shift(add(LHS, RHS));
constexpr int32_t EXPECTED_REGROUPED_RESULT = shift(shift(add(LHS, RHS)));

using Add = std::decay_t<decltype(add)>;
using Shift = std::decay_t<decltype(shift)>;
using AddThenShift = Chain<Add, Shift>;

/// Whether a chain can be split without being consumed
template <class T>
concept SplitsInPlace = requires(T &chain) { chain.split(); };

}  // namespace

static_assert(
    std::invocable<AddThenShift, int32_t, int32_t>,
    "a chain is called with the arguments of its first stage"
);

static_assert(
    EXPECTED_RESULT == AddThenShift(add, shift)(LHS, RHS),
    "a chain hands the first stage's result to the second"
);

static_assert(
    !std::invocable<AddThenShift, int32_t>,
    "a chain refuses a call missing an argument of its first stage"
);

static_assert(
    !std::invocable<AddThenShift, int32_t, int32_t, int32_t>,
    "a chain refuses a call with more arguments than its first stage takes"
);

/// Composing onto a chain nests the new stage on the right, however many stages there are
static_assert(
    std::same_as<
        decltype(AddThenShift(add, shift) | shift), Chain<Add, Chain<Shift, Shift>>>,
    "a stage composed onto a chain nests on the right"
);
static_assert(
    std::same_as<
        decltype(AddThenShift(add, shift) | shift | shift),
        Chain<Add, Chain<Shift, Chain<Shift, Shift>>>>,
    "every stage composed onto a chain nests on the right"
);

static_assert(
    EXPECTED_REGROUPED_RESULT == (AddThenShift(add, shift) | shift)(LHS, RHS),
    "a regrouped chain runs its stages in the order they were composed"
);

/// A chain can be taken apart into its two stages, which consumes it
static_assert(
    std::same_as<decltype(std::declval<AddThenShift>().split()), std::pair<Add, Shift>>,
    "splitting a chain hands back its first and second stages"
);
static_assert(
    !SplitsInPlace<AddThenShift>, "a chain that is not being consumed cannot be split"
);
