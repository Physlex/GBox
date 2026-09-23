//! This module provides unit testing for the `Thunk` type

#include <gtest/gtest.h>

import gbox.funky;

using fn::Thunk;

namespace {
/// Assert that it's impossible to construct a null thunk, thunks are always safe
static_assert(!std::is_constructible_v<Thunk<void>, std::nullptr_t>);

/// Assert that an empty closure defines a void returning thunk
static_assert(std::is_constructible_v<Thunk<void>, decltype([]() {})>);

int32_t add_two(int32_t a, int32_t b) { return a + b; }

/// Show a thunk being impossible to construct if it has a defined parameter
///
/// This is honestly the majority of why the thunk is valuable as it's own type. It
/// specifies that one can't just "have" a thunk, it genuinely needs to be a type
/// deriving from a parameter-less closure, by contract.
static_assert(
    !std::is_constructible_v<Thunk<void>, decltype([](int32_t a) { return a; })>
);

/// Show the construction of a parameterless thunk is possible
static_assert(std::is_constructible_v<Thunk<int>, decltype([]() { return 0; })>);

// Show that a closure wrapping a function with parameters is equivalent to a thunk
static_assert(
    std::is_constructible_v<Thunk<int>, decltype([]() { return add_two(1, 2); })>
);

/// Show that both functions are correctly hidden as "functions that return a given
/// type" when wrapped from the perpesective of a closure, despite being completely
/// different internally.
static_assert(
    std::is_same_v<decltype(Thunk<int>([]() { return 0; })), decltype(Thunk<int>([]() {
                       return add_two(1, 2);
                   }))>
);
}  // namespace
