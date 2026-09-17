//! This module provides unit testing for the `OnceFn` type

#include <gtest/gtest.h>

import gbox.func_ky;

using func::OnceFn;

namespace {
using Stateless = decltype(OnceFn());
using StatelessWithParams = decltype(OnceFn([](int32_t a, int32_t b) { return a + b; }));

/// Stateful representation of the
struct Stateful {
    std::int32_t n;
    constexpr auto operator()() const -> std::int32_t { return n; }
};

/// No matter what's provided to the once fn, it should only ever know of the closure
/// relative to "compiler wiring", unless a capture is provided.
static_assert(std::is_empty_v<Stateless>);
static_assert(std::is_empty_v<Stateless>);
static_assert(!std::is_empty_v<Stateful>);

/// obvious addition example that basically everyone uses by default
inline constexpr auto add = [](int32_t a, int32_t b) -> int32_t { return a + b; };
inline constexpr auto shift = [](int32_t value) { return value << 1; };

using Add = OnceFn<decltype(add)>;

/// Invoking a `OnceFn` consumes it, and does so at compile time.
static_assert(OnceFn(add)(1, 2) == 3);

/// Invoking a constant `OnceFn`, regardless of return type, is illegal
static_assert(!std::invocable<const Add &&, std::int32_t, std::int32_t>);

/// Only an rvalue can be invoked: consuming the closure requires moving out of it.
static_assert(std::invocable<Add &&, std::int32_t, std::int32_t>);
static_assert(!std::invocable<Add &, std::int32_t, std::int32_t>);
static_assert(!std::invocable<const Add &&, std::int32_t, std::int32_t>);

/// Demonstrates the construction of a pipeline in action
const int32_t expected_result = 6;
static_assert((OnceFn(add) | shift)(1, 2) == expected_result);
}  // namespace
