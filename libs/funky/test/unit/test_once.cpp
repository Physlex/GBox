//! This module provides unit testing for the `OnceFn` type

#include <gtest/gtest.h>

import gbox.funky;

using fn::once;
using fn::OnceFn;

namespace {

using Stateless = decltype(OnceFn());
using StatelessWithParams = decltype(OnceFn([](int32_t a, int32_t b) { return a + b; }));

struct Stateful {
    std::int32_t n;
    constexpr auto operator()() const -> std::int32_t { return n; }
};

}  // namespace

/// No matter what's provided to the once fn, it should only ever know of the closure
/// relative to "compiler wiring", unless a capture is provided.
static_assert(std::is_empty_v<Stateless>);
static_assert(std::is_empty_v<Stateless>);
static_assert(!std::is_empty_v<Stateful>);

/// obvious addition example that basically everyone uses by default
inline constexpr auto add = [](int32_t a, int32_t b) -> int32_t { return a + b; };

using Add = OnceFn<decltype(add)>;

/// Invoking a `OnceFn` consumes it, and does so at compile time.
static_assert(OnceFn(add)(1, 2) == 3);

/// Invoking a constant `OnceFn`, regardless of return type, is illegal
static_assert(!std::invocable<const Add &&, std::int32_t, std::int32_t>);

/// Only an rvalue can be invoked: consuming the closure requires moving out of it.
static_assert(std::invocable<Add &&, std::int32_t, std::int32_t>);
static_assert(!std::invocable<Add &, std::int32_t, std::int32_t>);
static_assert(!std::invocable<const Add &&, std::int32_t, std::int32_t>);

namespace {

/// Builds a once-callable from a named closure and consumes it, which is the only way one
/// can be invoked once it has a name of its own.
constexpr auto invoke_moved() -> std::int32_t {
    auto add_once = once(add);

    return std::move(add_once)(1, 2);
}

}  // namespace

/// A named closure is taken by copy, so the once-callable owns one rather than referring
/// back to where it was declared.
static_assert(
    std::same_as<decltype(once(add)), OnceFn<std::decay_t<decltype(add)>>>,
    "a once-callable built from a named closure stores it by value"
);

/// Owning the closure changes nothing about consuming it: a once-callable that has been
/// named can only be invoked by moving out of it.
static_assert(
    !std::invocable<decltype(once(add)) &, std::int32_t, std::int32_t>,
    "a named once-callable cannot be invoked without being moved from"
);
static_assert(
    invoke_moved() == 3,
    "a once-callable built from a named closure invokes the closure it copied"
);
