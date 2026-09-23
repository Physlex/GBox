//! This file implements testing for the contract between the once-callable and the chains
//! it composes into: a chain runs its operands in the order they were composed, and one
//! side of a composition needs to be chainable for the other to join it.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <cstdint>

import gbox.funky;

using fn::OnceFn;

namespace {

/// obvious addition example that basically everyone uses by default
inline constexpr auto add = [](int32_t a, int32_t b) -> int32_t { return a + b; };
inline constexpr auto shift = [](int32_t value) { return value << 1; };

/// Demonstrates the construction of a pipeline in action
const int32_t expected_result = 6;
static_assert((OnceFn(add) | shift)(1, 2) == expected_result);

}  // namespace
