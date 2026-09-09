//! This file implements unit testing for the static reference type

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>

import gbox.cell;

namespace {

/// Minimal type with a member to reach through a reference
struct Counter {
    int ticks = 0;

    void tick() { this->ticks += 1; }
};

}  // namespace

// A reference names a single holder of a static-lifetime value, passed on by move.
static_assert(std::is_move_constructible_v<cell::StaticRef<Counter>>);
static_assert(!std::is_copy_constructible_v<cell::StaticRef<Counter>>);

// A reference is only ever formed from storage that already holds a value, so there is no
// way to arrive at one which points at nothing.
static_assert(!std::is_default_constructible_v<cell::StaticRef<Counter>>);
static_assert(!std::is_constructible_v<cell::StaticRef<Counter>, std::nullptr_t>);
static_assert(!std::is_constructible_v<cell::StaticRef<Counter>, Counter *>);

/// Reach the referenced value through the dereference operator
TEST(staticRefDeref, staticRefTests) {
    auto held = cell::Static<Counter>();
    auto ref = cell::StaticRef<Counter>(held);

    (*ref).tick();

    ASSERT_EQ(1, (*ref).ticks);
}

/// Reach a member of the referenced value through the arrow operator
TEST(staticRefArrow, staticRefTests) {
    auto held = cell::Static<Counter>();
    auto ref = cell::StaticRef<Counter>(held);

    ref->tick();
    ref->tick();

    ASSERT_EQ(2, ref->ticks);
}

/// Borrow the referenced value directly
TEST(staticRefGet, staticRefTests) {
    auto held = cell::Static<Counter>();
    auto ref = cell::StaticRef<Counter>(held);

    ref.get().tick();

    ASSERT_EQ(1, ref.get().ticks);
}
