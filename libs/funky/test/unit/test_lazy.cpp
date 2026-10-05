//! This file implements unit testing for the `Lazy` type annotation.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <concepts>
#include <cstdint>
#include <type_traits>

import gbox.funky;

using fn::lazily;
using fn::Lazy;

namespace {

constexpr auto add = [](int32_t a, int32_t b) -> int32_t { return a + b; };

using LazyAdd = decltype(lazily(add));

}  // namespace

static_assert(
    std::same_as<LazyAdd, Lazy<std::decay_t<decltype(add)>>>,
    "annotating a callable wraps it once"
);

static_assert(
    std::same_as<decltype(lazily(lazily(add))), LazyAdd>,
    "annotating work that is already lazy leaves it unchanged"
);
