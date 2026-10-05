//! This file defines the closures shared across `funky`'s tests, so that each test names
//! the same shapes of work rather than declaring its own.

#pragma once

#include <cstdint>

namespace {

int32_t add_two(int32_t a, int32_t b) { return a + b; }

/// Work that takes nothing, captures nothing, and returns nothing
using Empty = decltype([]() {});

/// Work that captures nothing and returns a value
using Constant = decltype([]() { return 0; });

/// Work that captures state narrower than a pointer
using Captured = decltype([n = int32_t{2}]() { return add_two(n, n); });

/// Work that captures state exactly as wide as a pointer on a 64-bit host
using Wide = decltype([n = int64_t{2}]() { return n; });

/// A closure still waiting on a parameter, and so not yet work
using Parameterised = decltype([](int32_t a) { return a; });

/// Adds two integers
constexpr auto add = [](int32_t a, int32_t b) -> int32_t { return a + b; };

/// Doubles an integer by shifting it left one bit
constexpr auto shift = [](int32_t value) -> int32_t { return value << 1; };

}  // namespace
